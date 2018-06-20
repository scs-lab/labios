//
// Created by hdevarajan on 5/9/18.
//

#include <algorithm>
#include "task_scheduler_service.h"
#include "../common/external_clients/memcached_impl.h"
#include "../aetrio_system.h"

std::shared_ptr<task_scheduler_service> task_scheduler_service::instance = nullptr;

int task_scheduler_service::run() {
    int count=0, read_count=0, write_count=0;

    std::shared_ptr<distributed_queue> queue=aetrio_system::getInstance(service_i)->get_queue_client(CLIENT_TASK_SUBJECT);
    std::vector<task*> task_list=std::vector<task*>();
    Timer t=Timer();
    t.startTime();

    int status=-1;
    task* task_i= queue->subscribe_task(status);
    if(status!=-1 && task_i!= nullptr){
        count++;
        switch (task_i->t_type){
            case task_type::WRITE_TASK:{
                write_task *wt= static_cast<write_task *>(task_i);
                std::cout<< serialization_manager().serialize_task(wt) << std::endl;
                task_list.push_back(wt);
                write_count++;
                break;
            }
            case task_type::READ_TASK:{
                read_task *rt= static_cast<read_task *>(task_i);
                std::cout<< serialization_manager().serialize_task(rt) << std::endl;
                task_list.push_back(rt);
                read_count++;
                break;
            }
        }
    }
    while(!kill){
        usleep(10);//NOTE: Why usleep here?
        status=-1;
        task_i= queue->subscribe_task_with_timeout(status);
        if(status!=-1 && task_i!= nullptr){
            count++;
            switch (task_i->t_type){
                case task_type::WRITE_TASK:{
                    write_task *wt= static_cast<write_task *>(task_i);
                    std::cout<< serialization_manager().serialize_task(wt) << std::endl;
                    task_list.push_back(wt);
                    write_count++;
                    break;
                }
                case task_type::READ_TASK:{
                    read_task *rt= static_cast<read_task *>(task_i);
                    std::cout<< serialization_manager().serialize_task(rt) << std::endl;
                    task_list.push_back(rt);
                    read_count++;
                    break;
                }
                default:
                    std::cout <<"run(): Error in task type\n";
                    break;
            }
        }
        auto time_elapsed= t.endTimeWithoutPrint("");
        if(!task_list.empty() && (count>MAX_TASK ||
                                  time_elapsed>MAX_TASK_TIMER)){
            schedule_tasks(task_list,write_count,read_count);
            count=0;
            t.startTime();
            task_list.clear();
        }
    }
    return 0;
}

void task_scheduler_service::schedule_tasks(std::vector<task*> tasks,int write_count,int read_count) {
    solver_input input(write_count,MAX_WORKER_COUNT);
    //input.num_task=write_count;
    int write_index=0,read_index=0,actual_index=0;
    /*std::unordered_map<int,std::vector<task*>>
            worker_tasks_map=std::unordered_map<int,std::vector<task*>>();*/
    std::unordered_map<int,int> write_task_solver=std::unordered_map<int,int>();
    std::unordered_map<int,int> static_task_solver=std::unordered_map<int,int>();
    //int i=0;
    for(auto task_t:tasks){

        switch (task_t->t_type){
            case task_type::WRITE_TASK:{
                auto *wt= static_cast<write_task *>(task_t);
                if(wt->destination.worker==-1){
                    input.task_size[write_index]=wt->source.size;
                    write_task_solver.emplace(actual_index,write_index);
                    write_index++;
                }else{
                    /*
                     * update exiting file
                     */
                    static_task_solver.emplace(actual_index,wt->destination.worker-1);
                }
                break;
            }
            case task_type::READ_TASK:{
                auto *rt= static_cast<read_task *>(task_t);
                static_task_solver.emplace(actual_index,rt->source.worker-1);
                read_index++;
                break;
            }
            default:
                std::cout <<"schedule_tasks(): Error in task type\n";
                break;
        }
        actual_index++;
    }
    std::shared_ptr<distributed_hashmap> map=aetrio_system::getInstance(service_i)->map_server;
    std::vector<std::pair<int,int>> sorted_workers=std::vector<std::pair<int,int>>();
    int original_index=0;
    for(int worker_index=0;worker_index<MAX_WORKER_COUNT;worker_index++){
        std::string val=map->get(table::WORKER_CAPACITY,std::to_string(worker_index+1));
        sorted_workers.push_back(std::make_pair(atoi(val.c_str()),original_index++));
    }
    std::sort(sorted_workers.begin(), sorted_workers.end());
    int new_index=0;
    for(auto pair:sorted_workers){
        std::string val=map->get(table::WORKER_SCORE,std::to_string(pair.second+1));
        input.worker_score[new_index]=atoi(val.c_str());
        input.worker_capacity[new_index]=pair.first;
        input.worker_energy[new_index]=WORKER_ENERGY[pair.second];
        std::cout<<"worker:"<<pair.second+1<<" capacity:"<<input.worker_capacity[new_index]<<" score:"<<input.worker_score[new_index]<<std::endl;
        new_index++;
    }
    std::shared_ptr<solver> solver_i=aetrio_system::getInstance(service_i)->solver_i;
    solver_output output=solver_i->solve(input);
    for(int t=0;t<input.num_task;t++){
        if(output.solution[t]-1 < 0 || output.solution[t]-1 > MAX_WORKER_COUNT){
		std::cout<<"No Solution found"<<std::endl;
		return;
	}
	output.solution[t]=sorted_workers[output.solution[t]-1].second;
    }
    for(int task_index=0;task_index<tasks.size();task_index++){
        auto read_iter=static_task_solver.find(task_index);
        int worker_index=-1;
        if(read_iter==static_task_solver.end()){
            auto write_iter=write_task_solver.find(task_index);
            if(write_iter==write_task_solver.end()){
                printf("Error");
            }else{
                write_index=write_iter->second;
                worker_index=output.solution[write_index];
            }
        }else{
            worker_index=read_iter->second;
        }
        auto worker_task_iter=output.worker_task_map.find(worker_index);
        std::vector<task*> worker_tasks;
        if(worker_task_iter==output.worker_task_map.end()){
            worker_tasks=std::vector<task*>();
        }else{
            worker_tasks=worker_task_iter->second;
            output.worker_task_map.erase(worker_task_iter);
        }
        task* task_i=tasks[task_index];
        switch (task_i->t_type){
            case task_type::WRITE_TASK:{
                write_task *wt= static_cast<write_task *>(tasks[task_index]);
                worker_tasks.push_back(wt);
                break;
            }
            case task_type::READ_TASK:{
                read_task *rt= static_cast<read_task *>(tasks[task_index]);
                worker_tasks.push_back(rt);
                break;
            }
        }

        output.worker_task_map.emplace(worker_index,worker_tasks);
    }
    for (std::pair<int,std::vector<task*>> element : output.worker_task_map)
    {
        std::cout<<"add to worker:"<<element.first+1<<std::endl;
        std::shared_ptr<distributed_queue> queue=aetrio_system::getInstance(service_i)->get_worker_queue(element.first+1);
        for(auto task:element.second){
            switch (task->t_type){
                case task_type::WRITE_TASK:{
                    write_task *wt= static_cast<write_task *>(task);
                    queue->publish_task(wt);
                    break;
                }
                case task_type::READ_TASK:{
                    read_task *rt= static_cast<read_task *>(task);
                    queue->publish_task(rt);
                    break;
                }
            }

        }
    }
}
