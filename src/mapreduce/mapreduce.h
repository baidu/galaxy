// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <string>

class MapInput {
public:
    std::string value() const;
};

class Mapper {
public:
	virtual void Map(const MapInput& input) = 0;
	virtual void Emit(const std::string& output, int num) = 0;
};

class ReduceInput {
public:
    std::string value();
	bool done();
	void NextValue();
};
class Reducer {
public:
	virtual void Emit(const std::string& output) = 0;
};

class MapReduceInput {
public:
	void set_format(const std::string& format);
	void set_filepattern(const std::string& pattern);
	void set_mapper_class(const std::string& mapper);
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
	MapReduceInput* add_input() { return new MapReduceInput(); }
	MapReduceOutput* output() { return new MapReduceOutput(); }
	void set_machines(int machines) { machines_ = machines; }
	int machines() const { return machines_; }
	void set_map_megabytes(int megabytes) {}
	void set_reduce_megabytes(int megabytes) {};
private:
	int machines_;
};

class MapReduceResult {
public:
	int machines_used();
	int time_taken(); 
};

#define REGISTER_MAPPER(mapper)
#define REGISTER_REDUCER(reducer)

bool MapReduce(const MapReduceSpecification& spec, 
		       MapReduceResult* result);

