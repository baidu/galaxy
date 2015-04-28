// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "mapreduce.h"

#include <gflags/gflags.h>
#include <iostream>
#include <galaxy.h>

DECLARE_string(role);

/// MapInput implement
std::string MapInput::value() const {
    return data_[idx_];
}

bool MapInput::done() {
    return idx_ >= data_.size();
}

void MapInput::NextValue() {
    ++idx_;
}

MapInput::MapInput()
  : idx_(0) {
    std::string tmp;
    while (std::cin >> tmp) {
        data_.push_back(tmp);
    }
}

/// ReduceInput implement
ReduceInput::ReduceInput()
  : key_idx_(0), value_idx_(0) {
    std::string key;
    std::string value;
    std::string last_key = "";
    int32_t key_num = 0;
    while (std::cin >> key >> value) {
        if (key != last_key) {
            keys_.push_back(key);
            values_.push_back(std::vector<std::string>());
            key_num ++;
            last_key = key;
        }
        values_[key_num - 1].push_back(value);
    }
}

std::string ReduceInput::key() {
    return keys_[key_idx_];
}
std::string ReduceInput::value() {
    return values_[key_idx_][value_idx_];
}
bool ReduceInput::done() {
    return value_idx_ >= values_[key_idx_].size();
}
void ReduceInput::NextValue() {
    value_idx_++;
}
void ReduceInput::NextKey() {
    key_idx_++;
    value_idx_ = 0;
}
bool ReduceInput::alldone() {
    return key_idx_ >= keys_.size();
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


void Mapper::Emit(const std::string& output, int num) {
    std::cout << output << "\t" << num << std::endl;
}

void Reducer::Emit(const std::string& output) {
    std::cout << output << std::endl;
}

Mapper* g_mapper = NULL;
Reducer* g_reducer = NULL;
const MapReduceSpecification* g_spec = NULL;


int64_t GalaxyNewJob(galaxy::Galaxy* galaxy,
                  const std::string& job_name, const std::string& tarfile,
                  const std::string& cmd_line, int replica) {
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
    job.cpu_share = 1;
    job.mem_share = 1024*1024*1024;
    return galaxy->NewJob(job);
}

bool MapReduceMaster(const MapReduceSpecification& spec, MapReduceResult* result) {

    galaxy::Galaxy* cluster = galaxy::Galaxy::ConnectGalaxy("yq01-tera60.yq01:8101");
    std::vector<galaxy::JobInstanceDescription> jobs;
    /// map
    int64_t mapper_id = 
        GalaxyNewJob(cluster, "mapper", "mapper.tar.gz", "./mapper.sh", spec.machines());
    bool done = false;
    fprintf(stderr, "Mapper Start\n");
    while(!done) {
        cluster->ListJob(&jobs);
        for (uint32_t i = 0 ; i < jobs.size(); i++) {
            if (jobs[i].job_id == mapper_id && jobs[i].replicate_count == 0) {
                done = true;
                break;
            }
        }
        jobs.clear();
        fprintf(stderr, ".");
        sleep(1);
    }
    fprintf(stderr, "\nMapper done\n");
    /// reduce
    int64_t reducer_id = 
        GalaxyNewJob(cluster, "reducer", "reducer.tar.gz", "./reducer.sh", spec.machines());
    done = false;
    fprintf(stderr, "Reducer Start\n");
    while(!done) {
        cluster->ListJob(&jobs);
        for (uint32_t i = 0; i < jobs.size(); i++) {
            if (jobs[i].job_id == reducer_id && jobs[i].replicate_count == 0) {
                done = true;
                break;
            }
        }
        jobs.clear();
        fprintf(stderr, ".");
        sleep(1);
    }
    fprintf(stderr, "\nReducer done\n");
    return true;
}

bool Map(const MapReduceSpecification& spec, MapReduceResult* result) {
    MapInput input;
    while(!input.done()) {
        g_mapper->Map(input);
        input.NextValue();
    }
    return true;
}

bool Reduce(const MapReduceSpecification& spec, MapReduceResult* result) {
    ReduceInput input;
    while(!input.alldone()) {
        std::cout << input.key() << "\t";
        g_reducer->Reduce(&input);
        input.NextKey();
    }
    return true;
}

bool Client(const MapReduceSpecification& spec, MapReduceResult* result) {
    return true;
}

bool MapReduce(const MapReduceSpecification& spec, MapReduceResult* result) {
    if (FLAGS_role == "master") {
        return MapReduceMaster(spec,result);
    } else if (FLAGS_role == "mapper") {
        return Map(spec,result);
    } else if (FLAGS_role == "reducer") {
        return Reduce(spec,result);
    } else {
        /// Client
        return Client(spec,result);
    }
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
