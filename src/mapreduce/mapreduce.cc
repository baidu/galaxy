// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "mapreduce.h"

#include "galaxy.h"

std::string MapInput::value() const {
    return "";
}

std::string ReduceInput::value() {
    return "";
}
bool ReduceInput::done() {
    return true;
}
void ReduceInput::NextValue() {
}



void MapReduceInput::set_format(const std::string& format) {
}
void MapReduceInput::set_filepattern(const std::string& pattern) {
}
void MapReduceInput::set_mapper_class(const std::string& mapper) {
}


void MapReduceOutput::set_filebase(const std::string& filebase) {
}
void MapReduceOutput::set_num_tasks(int task_num) {
}
void MapReduceOutput::set_format(const std::string& format) {
}
void MapReduceOutput::set_reducer_class(const std::string& reducer) {
}
void MapReduceOutput::set_combiner_class(const std::string& combiner) {
}



int MapReduceResult::machines_used() { return 0; }
int MapReduceResult::time_taken() { return 0; } 

bool GalaxyNewJob(const std::string& job_name, const std::string& tarfile,
                  const std::string& cmd_line, int replica) {
    galaxy::Galaxy* galaxy = galaxy::Galaxy::ConnectGalaxy("localhost:8102");
    galaxy::JobDescription job;
    galaxy::PackageDescription pkg;

    std::string task_raw;
    FILE* fp = fopen(tarfile.c_str(), "r");
    if (fp == NULL) {
        fprintf(stderr, "Open %s for read fail\n", tarfile.c_str());
        return -2;
    }
    char buf[1024];
    int len = 0;
    while ((len = fread(buf, 1, 1024, fp)) > 0) {
        task_raw.append(buf, len);
    }
    fclose(fp);
    printf("Job binary len %lu\n", task_raw.size());

    pkg.source = task_raw;
    job.pkg = pkg;
    job.cmd_line = cmd_line;
    job.replicate_count = replica;
    job.job_name = job_name;
    return galaxy->NewJob(job);
}

bool MapReduce(const MapReduceSpecification& spec, MapReduceResult* result) {
    GalaxyNewJob("mapper", "mapper.tar.gz", "./mapper", spec.machines());
    GalaxyNewJob("reducer", "reducer.tar.gz", "./reducer", spec.machines());
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
