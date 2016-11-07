/***************************************************************************
 * 
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file container_description.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/11/03 18:59:57
 * @brief 
 *  
 **/

#include "ins_sdk.h"
#include "protocol/appmaster.pb.h"
#include "boost/shared_ptr.hpp"
#include <gflags/gflags.h>
#include <stdio.h>
#include <iostream>

DEFINE_string(nexus_addr, "", "");
DEFINE_string(nexus_path, "", "");

void PrintHelp(const char* argv0);

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    boost::shared_ptr<galaxy::ins::sdk::InsSDK> nexus(new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_addr));

    const std::string start_key = FLAGS_nexus_path;
    const std::string end_key = FLAGS_nexus_path + "~";

    ::galaxy::ins::sdk::ScanResult* result
        = nexus->Scan(start_key, end_key);
    std::cerr << "====================================" << FLAGS_nexus_addr << "\n";

    while (!result->Done()) {
        const std::string& value = result->Value();

        baidu::galaxy::proto::JobInfo job;
        if (job.ParseFromString(value)) {
            baidu::galaxy::proto::PodDescription pod = job.desc().pod();
            
            for (int i = 0; i < pod.tasks_size(); i++) {
                if (pod.tasks(i).services_size() > 0 
                            && pod.tasks(i).services(0).has_use_bns()
                            && pod.tasks(i).services(0).use_bns()) {
                    fprintf(stdout, "%s %s %s\n", job.jobid().c_str(),
                                pod.tasks(i).services(0).service_name().c_str(),
                                pod.tasks(i).services(0).token().c_str());
                    break;
                }
            }
        }

        result->Next();
    }
    delete result;

    return 0;
}


void PrintHelp(const char* argv0) {
    std::cout << "usage: " << argv0 << " --nexus_addr=xx nexus_path=xx start_key=xx end_key=xx\n";
}

















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
