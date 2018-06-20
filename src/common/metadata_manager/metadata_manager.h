//
// Created by hariharan on 2/16/18.
//

#ifndef AETRIO_MAIN_METADATA_MANAGER_H
#define AETRIO_MAIN_METADATA_MANAGER_H


#include <cstdio>
#include <string>
#include <unordered_map>
#include <cereal/types/memory.hpp>
#include "../../aetrio_system.h"
#include "../data_structures.h"

class metadata_manager {
private:
    static std::shared_ptr<metadata_manager> instance;
    std::unordered_map<FILE*,std::string> fh_map;
    std::unordered_map<std::string,file_stat> file_map;
    service service_i;
    metadata_manager(service service):fh_map(),file_map(),service_i(service){}
public:
    inline static std::shared_ptr<metadata_manager> getInstance(service service){
        return instance== nullptr ? instance=std::shared_ptr<metadata_manager>(new metadata_manager(service))
                                  : instance;
    }
    bool is_created(std::string filename);
    int create(std::string filename,std::string mode,FILE* &fh);
    bool is_opened(std::string filename);
    bool is_opened(FILE* fh);
    int update_on_open(std::string filename,std::string mode,FILE* &fh);
    int update_on_close(FILE* &fh);
    int remove_chunks(std::string basic_string);

    std::string get_filename(FILE* fh);
    std::size_t get_filesize(std::string basic_string);
    std::string get_mode(std::string basic_string);
    std::size_t get_fp(std::string basic_string);
    int update_on_seek(std::string basic_string,size_t offset, size_t origin);
    int update_read_task_info(std::vector<read_task> task_k,std::string filename);
    int update_write_task_info(std::vector<write_task> task_ks,std::string filename);
    int update_write_task_info(write_task task_ks,std::string filename);
    std::vector<chunk_meta> fetch_chunks(read_task task);
    void update_on_read(std::string filename, size_t size);

    void update_on_write(std::string filename, size_t size);

};


#endif //AETRIO_MAIN_METADATA_MANAGER_H
