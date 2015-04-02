// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "mapreduce.h"

#include <iostream>

#include <galaxy.h>

std::string MapInput::value() const {
    return data_[idx_++];
}

bool MapInput::done() {
    return idx_ >= data_.size();
}

MapInput::MapInput()
  : idx_(0) {
    std::string tmp;
    while (std::cin >> tmp) {
        data_.push_back(tmp);
    }
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


MapReduceInput* MapReduceSpecification::add_input() {
    int32_t n = inputs_.size();
    inputs_.resize(n + 1);
    return &inputs_[n];
}
MapReduceOutput* MapReduceSpecification::output() {
    return &output_;
}

void MapReduceSpecification::set_machines(int machines) {
    machines_ = machines;
}
int MapReduceSpecification::machines() const {
    return machines_;
}
void MapReduceSpecification::set_map_megabytes(int megabytes) {
}
void MapReduceSpecification::set_reduce_megabytes(int megabytes) {
};

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


void Mapper::Emit(const std::string& output, int num) {
    std::cout << output << "\t" << num << std::endl;
}

void Reducer::Emit(const std::string& output) {
}

Mapper* g_mapper = NULL;
Reducer* g_reducer = NULL;
const MapReduceSpecification* g_spec = NULL;

bool MapReduce(const MapReduceSpecification& spec, MapReduceResult* result) {
    std::string map_cmd = 
        "bfs_client cat /galaxy/src/$task_id.cc | ./mapper | ./shuffle";
    /// map
    GalaxyNewJob("mapper", "mapper.tar.gz", map_cmd, spec.machines());
    /// combine

    /// shuffle
    system("./shuffle");
    /// reduce
    GalaxyNewJob("reducer", "reducer.tar.gz", "./reducer", spec.machines());
    return true;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
