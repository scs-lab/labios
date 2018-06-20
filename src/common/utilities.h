//
// Created by hariharan on 2/23/18.
//

#ifndef AETRIO_MAIN_UTILITY_H
#define AETRIO_MAIN_UTILITY_H

#include <string>
#include <vector>
#include <cstring>
#include <getopt.h>
#include "configuration_manager.h"

static std::vector<std::string> string_split(std::string value,std::string delimiter=","){
    char *token = strtok(const_cast<char *>(value.c_str()), delimiter.c_str());
    std::vector<std::string> splits=std::vector<std::string>();
    while (token != NULL)
    {
        splits.push_back(token);
        token = strtok(NULL, delimiter.c_str());
    }
    return splits;
}
static int parse_opts(int argc, char *argv[]){
    auto conf=configuration_manager::get_instance();
    int flags, opt;
    int nsecs, tfnd;

    nsecs = 0;
    tfnd = 0;
    flags = 0;
    while ((opt = getopt (argc, argv, "qc:qs:mc:ms:")) != -1)
    {
        switch (opt)
        {
            case 'qc':{
                conf->NATS_URL_CLIENT=std::string(optarg);
                break;
            }
            case 'qs':{
                conf->NATS_URL_SERVER=std::string(optarg);
                break;
            }
            case 'mc':{
                conf->MEMCACHED_URL_CLIENT=std::string(optarg);
                break;
            }
            case 'ms':{
                conf->MEMCACHED_URL_SERVER=std::string(optarg);
                break;
            }
            default:               /* '?' */
                fprintf (stderr, "Usage: %s [-qc nats client URL] [-qs nats server URL] [-mc memcached client URL] [-ms memcached server URL]\n", argv[0]);
                exit (EXIT_FAILURE);
        }
    }
    return 0;
}

#endif //AETRIO_MAIN_UTILITY_H
