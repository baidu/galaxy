// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/utils.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "./agent_utils_test dir prefix max_deep\n"); 
        return EXIT_FAILURE;
    }

    std::string path(argv[1]);
    bool is_dir;
    if (galaxy::file::IsDir(path, is_dir)) {
        if (is_dir) {
            fprintf(stdout, "%s is dir\n", path.c_str()); 
        } else {
            fprintf(stdout, "%s isn't dir\n", path.c_str()); 
            return EXIT_FAILURE;
        }
    }

    std::vector<std::string> files;
    std::string prefix(argv[2]);
    int max_deep = atoi(argv[3]);
    galaxy::file::GetDirFilesByPrefix(path, prefix, &files, max_deep);
    std::vector<std::string>::iterator it;
    for (it = files.begin(); it != files.end(); ++it) {
        fprintf(stdout, "%s found\n", it->c_str());
    }
    return EXIT_SUCCESS;
} 

/* vim: set ts=4 sw=4 sts=4 tw=100 */
