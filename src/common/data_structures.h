/*******************************************************************************
* Created by hariharan on 2/17/18.
* Updated by akougkas on 6/26/2018
* Illinois Institute of Technology - SCS Lab
* (C) 2018
******************************************************************************/
#ifndef AETRIO_MAIN_STRUCTURE_H
#define AETRIO_MAIN_STRUCTURE_H
/******************************************************************************
*include files
******************************************************************************/
#include <cereal/types/memory.hpp>
#include "enumerations.h"
#include "constants.h"
#include <cereal/types/string.hpp>
#include <cereal/types/common.hpp>
#include <utility>
/******************************************************************************
*message_key structure
******************************************************************************/
struct message_key{
    message_type m_type;
    map_type mp_type;
    operation operation_type;
    char key[KEY_SIZE];
};
/******************************************************************************
*message structure
******************************************************************************/
struct message{
    message_type m_type;
    map_type mp_type;
    char key[KEY_SIZE];
    size_t data_size;
    char* data;
};
/******************************************************************************
*file structure
******************************************************************************/
struct file{
    location_type location;
    std::string filename;
    int64_t offset;
    std::size_t size;
    int worker=-1;
/*******************
*Constructors
*******************/
    file(std::string filename, int64_t offset, std::size_t file_size)
            :filename(std::move(filename)),offset(offset),size(file_size),
             location(CACHE){}
    file(const file &file_t)
            :filename(file_t.filename),offset(file_t.offset),size(file_t.size),
             location(file_t.location),worker(file_t.worker){}
    file() : location(CACHE), filename(""), offset(0), size(0){}
/*******************
*Destructor
*******************/
    virtual ~file() {}
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive( this->filename,this->offset,this->size,this->location,this->worker);
    }
};
/******************************************************************************
*chunk_meta structure
******************************************************************************/
struct chunk_meta{
    file actual_user_chunk;
    file destination;
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive( this->actual_user_chunk,this->destination );
    }

};
/******************************************************************************
*chunk_msg structure
******************************************************************************/
struct chunk_msg{
    location_type chunkType;
    std::string dataspace_id;
    std::string filename;
    size_t offset;
    size_t file_size;
};
/******************************************************************************
*file_meta structure
******************************************************************************/
struct file_meta{
    file file_struct;
    std::vector<chunk_meta> chunks;
/*******************
*Constructors
*******************/
    file_meta():chunks(){}
/*******************
*Destructor
*******************/
    virtual ~file_meta() {}
};
/******************************************************************************
*dataspace structure
******************************************************************************/
struct dataspace{
    size_t size;
    void* data;
};
/******************************************************************************
*file_stat structure
******************************************************************************/
struct file_stat{
    FILE* fh;
    std::size_t file_pointer;
    std::size_t file_size;
    std::string mode;
    bool is_open;
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(this->file_pointer, this->file_size,this->mode,this->is_open );
    }
};
/******************************************************************************
*task structure
******************************************************************************/
struct task {
    task_type t_type;
    int64_t task_id=0;
/*******************
*Constructors
*******************/
    explicit task(task_type t_type):t_type(t_type){}
    task(const task &t_other)
            :t_type(t_other.t_type),task_id(t_other.task_id){}
/*******************
*Destructor
*******************/
    virtual ~task() {}
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(this->t_type,this->task_id);
    }
};
/******************************************************************************
*write_task structure
******************************************************************************/
struct write_task:public task{
    file source;
    file destination;
    bool meta_updated = false;
/*******************
*Constructors
*******************/
    write_task():task(task_type::WRITE_TASK){}
    write_task(const file &source, const file &destination)
            :task(task_type::WRITE_TASK),source(source),destination(destination){}
    write_task(const write_task &write_task_t)
            :task(task_type::WRITE_TASK), source(write_task_t.source),
                                        destination(write_task_t.destination){}
/*******************
*Destructor
*******************/
    virtual ~write_task() {}
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive( this->t_type,this->task_id,this->source,
                 this->destination,this->meta_updated);
    }
};
/******************************************************************************
*read_task structure
******************************************************************************/
struct read_task:public task{
    file source;
    file destination;
    bool meta_updated = false;
    bool local_copy = false;
/*******************
*Constructors
*******************/
    read_task(const file &source, const file &destination)
            :task(task_type::READ_TASK),source(source),destination(destination){}
    read_task():task(task_type::READ_TASK){}
    read_task(const read_task &read_task_t)
            :task(task_type::READ_TASK), source(read_task_t.source),
                                        destination(read_task_t.destination){}
/*******************
*Destructor
*******************/
    virtual ~read_task() {}
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive( this->t_type,this->task_id,this->source,
                 this->destination,this->meta_updated,this->local_copy);
    }
};
/******************************************************************************
*flush_task structure
******************************************************************************/
struct flush_task:public task{
    file source;
    file destination;
/*******************
*Constructors
*******************/
    flush_task():task(task_type::FLUSH_TASK){}
    flush_task(const flush_task &flush_task_t):task(task_type::FLUSH_TASK),
            source(flush_task_t.source),
            destination(flush_task_t.destination){}
    flush_task(file source,
            file destination,
            location_type dest_t,
            std::string datasource_id):task(task_type::FLUSH_TASK),source(source),destination(destination){}
/*******************
*Destructor
*******************/
    virtual ~flush_task() {}
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(this->t_type,this->task_id,this->source,this->destination);
    }
};
/******************************************************************************
*delete_task structure
******************************************************************************/
struct delete_task:public task{
    file source;
/*******************
*Constructors
*******************/
    delete_task():task(task_type::DELETE_TASK){}
    delete_task(const delete_task &delete_task_i)
            :task(task_type::DELETE_TASK),source(delete_task_i.source){}
    explicit delete_task(file &source)
            :task(task_type::DELETE_TASK),source(source){}
/*******************
*Destructor
*******************/
    virtual ~delete_task() {}
/*******************
*Serialization
*******************/
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(this->t_type,this->task_id,this->source);
    }
};
/******************************************************************************
*solver_input structure
******************************************************************************/
struct solver_input{
    std::vector<task*> tasks;
    int num_tasks;
    int64_t *task_size;
    int64_t total_io_size=0;
/*******************
*Constructors
*******************/
    explicit solver_input(std::vector<task*> &task_list, int num_tasks){
        this->tasks = task_list;
        this->num_tasks = num_tasks;
        task_size=new int64_t[num_tasks];
    }
/*******************
*Destructor
*******************/
    virtual ~solver_input() {}
};
/******************************************************************************
*solver_output structure
******************************************************************************/
struct solver_output{
    int* solution;
    //workerID->list of tasks
    std::unordered_map<int,std::vector<task*>> worker_task_map;
/*******************
*Constructors
*******************/
    explicit solver_output(int num_task):worker_task_map(){
        solution=new int[num_task];
    }
/*******************
*Destructor
*******************/
    virtual ~solver_output() {}
};

#endif //AETRIO_MAIN_STRUCTURE_H
