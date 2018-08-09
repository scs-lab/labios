/******************************************************************************
*include files
******************************************************************************/
#include <mpi.h>
#include "memcached_impl.h"

/******************************************************************************
*Interface
******************************************************************************/
int MemcacheDImpl::put(const table &name, std::string key, const std::string &value,std::string group_key) {
    key=std::to_string(name)+KEY_SEPARATOR+key;
    if(group_key=="-1"){
        group_key=get_server(key);
    }
    memcached_return_t rc= memcached_set_by_key(mem_client,
                                                group_key.c_str(),
                                                group_key.length(),
                                                key.c_str(),
                                                key.length(),
                                                value.c_str(),
                                                value.length()+1,
                                                (time_t)0,
                                                (uint32_t)0);
    if(rc!=MEMCACHED_SUCCESS) std::cerr<< "put failed for key:"<<key<<"\n";
    return rc;
}

std::string MemcacheDImpl::get(const table &name, std::string key,std::string group_key) {
    char *return_value;
    size_t size;
    key=std::to_string(name)+KEY_SEPARATOR+key;
    if(group_key=="-1"){
        group_key=get_server(key);
    }
    return_value = memcached_get_by_key(mem_client,
                                        group_key.c_str(),
                                        group_key.length(),
                                         key.c_str(),
                                         key.length(),
                                         &size,
                                         (time_t)0,
                                         (uint32_t)0);
    if(return_value== NULL){
        return "";
    }
    return return_value;
}

std::string MemcacheDImpl::remove(const table &name, std::string key,std::string group_key) {
    key=std::to_string(name)+KEY_SEPARATOR+key;
    if(group_key=="-1"){
        group_key=get_server(key);
    }
    memcached_delete_by_key(mem_client,group_key.c_str(),
                            group_key.length(), key.c_str(), key.length(),
                            (time_t)0);
    return "";
}

bool MemcacheDImpl::exists(const table &name, std::string key,std::string group_key) {
    key=std::to_string(name)+KEY_SEPARATOR+key;

    if(group_key=="-1"){
        group_key=get_server(key);
    }
    memcached_return_t rc= memcached_exist_by_key(mem_client,
            group_key.c_str(),
            group_key.length(),
            key.c_str(),
            key.length());
    return rc == memcached_return_t::MEMCACHED_SUCCESS;
}

bool MemcacheDImpl::purge() {
    memcached_flush(mem_client,0);
}

size_t MemcacheDImpl::counter_init(const table &name, std::string key,
                                   std::string group_key) {
    key=std::to_string(name)+KEY_SEPARATOR+key;
    if(group_key=="-1"){
        group_key=get_server(key);
    }
    return distributed_hashmap::counter_init(name, key, group_key);
}

size_t MemcacheDImpl::counter_inc(const table &name, std::string key,
                                  std::string group_key) {
    key=std::to_string(name)+KEY_SEPARATOR+key;
    if(group_key=="-1"){
        group_key=get_server(key);
    }
    size_t value=0;
    memcached_return_t rc= memcached_increment(mem_client,key.c_str(),key.length(),1,&value);
    if(rc==MEMCACHED_NOTFOUND){
        memcached_set_by_key(mem_client,group_key.c_str(),group_key.length(),
                key.c_str(),key
        .length(),
                "0",1,0,0);
        value=0;
    }
    return value;
}

std::string MemcacheDImpl::get_server(std::string key) {
    auto hash=CityHash64(key.c_str(),key.length());
    size_t server=hash%num_servers;
    return std::to_string(server);
}

