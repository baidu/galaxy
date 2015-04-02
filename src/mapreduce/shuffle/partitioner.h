#ifndef GALAXY_MAPREDUCE_SHUFFLE_PARTITIONER_H
#define GALAXY_MAPREDUCE_SHUFFLE_PARTITIONER_H

#include <stdint.h>
#include <assert.h>
#include <string>

class Partitioner {
public:
	void SetReducerTotal(uint32_t reducer_total) {
		reducer_total_ =  reducer_total;	
		assert(reducer_total_ > 0);	
	}
	uint32_t GetReducerSlot(const std::string& key) {
		uint32_t number = KeyToNumber(key);
		return number % reducer_total_;
	}
	virtual uint32_t KeyToNumber(const std::string& key) = 0;
private:
	uint32_t reducer_total_;
};


class HashPartitioner : public Partitioner {
public:
	virtual uint32_t KeyToNumber(const std::string& key) {
		size_t key_len = key.size();
		uint32_t hash = 0;
		for (size_t i = 0; i < key_len; i++) {
			hash = 31 * hash + (size_t)key[i];
		}
		return hash;
	}
};


#endif
//GALAXY_MAPREDUCE_SHUFFLE_PARTITIONER_H


