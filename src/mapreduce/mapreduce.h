// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <string>
#include <vector>

class MapInput {
public:
    std::string value() const;
public:
    MapInput();
    bool done();
private:
    std::vector<std::string> data_;
    mutable uint32_t idx_;
};

class Mapper {
public:
    virtual void Map(const MapInput& input) = 0;
    virtual void Emit(const std::string& output, int num);
};

class ReduceInput {
public:
    std::string value();
    bool done();
    void NextValue();
};

class Reducer {
public:
    virtual void Reduce(ReduceInput* input) = 0;
    virtual void Emit(const std::string& output);
};

class MapReduceInput {
public:
	void set_format(const std::string& format);
	void set_filepattern(const std::string& pattern);
	void set_mapper_class(const std::string& mapper);
public:
    std::string get_file() const { return file_;}
private:
    std::string file_;
};

class MapReduceOutput {
public:
	void set_filebase(const std::string& filebase);
	void set_num_tasks(int task_num);
	void set_format(const std::string& format);
	void set_reducer_class(const std::string& reducer);
	void set_combiner_class(const std::string& combiner);
};

class MapReduceSpecification {
public:
	MapReduceInput* add_input();
	MapReduceOutput* output();
	void set_machines(int machines);
	int machines() const;
	void set_map_megabytes(int megabytes);
	void set_reduce_megabytes(int megabytes);
private:
	int machines_;
    std::vector<MapReduceInput> inputs_;
    MapReduceOutput output_;
};

class MapReduceResult {
public:
	int machines_used();
	int time_taken(); 
};

extern Mapper* g_mapper;
#define REGISTER_MAPPER(mapper) \
    mapper __mapper;    \
    class Reg##mapper { \
    public: \
    Reg##mapper() { \
        g_mapper = &__mapper; \
    } \
    }; \
    Reg##mapper __regmapper;

extern Reducer* g_reducer;
#define REGISTER_REDUCER(reducer) \
    reducer __reducer;    \
    class Reg##reducer { \
    public: \
    Reg##reducer() { \
        g_reducer = &__reducer; \
    } \
    }; \
    Reg##reducer ____regreducer;

bool MapReduce(const MapReduceSpecification& spec, 
		       MapReduceResult* result);

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
