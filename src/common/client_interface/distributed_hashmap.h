//
// Created by hariharan on 2/23/18.
//

#ifndef PORUS_MAIN_DISTRIBUTEDHASHMAP_H
#define PORUS_MAIN_DISTRIBUTEDHASHMAP_H


#include <cereal/types/memory.hpp>
#include "../enumerations.h"
#include "../constants.h"
#include "../exceptions.h"

class distributed_hashmap {
protected:
    Service service;
public:
    distributed_hashmap(Service service):service(service){

    }
    virtual int put(table table_name,std::string key,std::string value){
        throw NotImplementedException("put");
    }
    virtual std::string get(table table_name, std::string key){
        throw NotImplementedException("get");
    }
    virtual std::string remove(table table_name, std::string key){
        throw NotImplementedException("remove");
    }
};


#endif //PORUS_MAIN_DISTRIBUTEDHASHMAP_H