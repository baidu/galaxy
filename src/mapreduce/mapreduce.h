
#include <string>

class MapInput {
	const std::string& value();
};
class Mapper {
	virtual void Map(const MapInput& input) = 0;
	virtual void Emit(const std::string& output) = 0;
};

class ReduceInput {
	const std::string& value();
	bool done();
	void NextValue();
};
class Reducer {
	virtual void Emit(const std::string& output) = 0;
};

class MapReduceInput {
	void set_format(const std::string& format);
	void set_filepattern(const std::string& pattern);
	void set_mapper_class(const std::string& mapper);
};

class MapReduceOutput {
	void set_filebase(const std::string& filebase);
	void set_num_tasks(int task_num);
	void set_format(const std::string& format);
	void set_reducer_class(const std::string& reducer);
	void set_combiner_class(const std::string& combiner);
};

class MapReduceSpecification {
	MapReduceInput* add_input();
	MpaReduceOutput* output();
	void set_machines(int machines);
	void set_map_megabytes(int megabytes);
	void set_reduce_megabytes(int megabytes);
};

class MapReduceResult {
	int machines_used();
	int time_taken(); 
};

bool MapReduce(const MapReduceSpecification& spec, 
		       MapReduceResult* result);

