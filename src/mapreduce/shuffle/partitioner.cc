// Copyright (C) 2015, Baidu Inc.
// Author: Sun Junyi (sunjunyi01@baidu.com)

#include "partitioner.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "partitioner [reducer total] \n");
		return 1;
	}
	std::string record;
	uint32_t reducer_total = atoi( argv[1] );
	Partitioner* partition = new HashPartitioner();
	partition->SetReducerTotal(reducer_total);

	while (getline(std::cin, record)) {
		size_t key_span = strcspn(record.c_str(), "\t "); //tab or whitespace
		std::string key(record.c_str(), key_span);
		uint32_t reducer_slot = partition->GetReducerSlot(key.c_str());
		std::cout << reducer_slot << "\1\1\1" << record << std::endl;
	}
	return 0;
}
