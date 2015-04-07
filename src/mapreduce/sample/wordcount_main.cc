// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <mapreduce/mapreduce.h>

int main(int argc, char** argv) {
    //ParseCommandLineFlags(argc, argv);
    
    MapReduceSpecification spec;
    
    // Store list of input files into spec
    for (int i = 1; i < argc; i++) {
        MapReduceInput* input = spec.add_input();
        input->set_format("text");
        input->set_filepattern(argv[i]);
        input->set_mapper_class("WordCounter");
    }
    
    // Specify the output files:
    // /gfs/test/freq-00000-of-00100
    // /gfs/test/freq-00001-of-00100
    // 
    MapReduceOutput* out = spec.output();
    out->set_filebase("/bfs/test/freq");
    out->set_num_tasks(100);
    out->set_format("text");
    out->set_reducer_class("Adder");
    
    // Optional: do partial sums within map
    // tasks to save network bandwidth
    out->set_combiner_class("Adder");
    
    // Tuning parameters: use at most 2000
    // machines and 100 MB of memory per task
    spec.set_machines(13);
    spec.set_map_megabytes(100);
    spec.set_reduce_megabytes(100);
    
    // Now run it
    MapReduceResult result;
    if (!MapReduce(spec, &result)) abort();
    
    // Done: result structure contains info
    // about counters, time taken, number of
    // machines used, etc.
    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
