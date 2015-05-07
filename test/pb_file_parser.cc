// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "pb_file_parser.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


namespace galaxy {
namespace test {

static int PB_BUF_SIZE = 1024 * 1000;
static int OPEN_READ_FLAGS = O_RDONLY;

static int OPEN_WRITE_FLAGS = O_WRONLY | O_CREAT;
static int OPEN_WRITE_MODE = S_IRWXU;
    
bool DumpToFile(const std::string& file, const google::protobuf::Message& pb) {
    std::string data_buffer;
    if (!pb.SerializeToString(&data_buffer)) {
        fprintf(stderr, "serialize to string failed\n");
        return false; 
    }

    int fout = open(file.c_str(), OPEN_WRITE_FLAGS, OPEN_WRITE_MODE);
    if (fout == -1) {
        fprintf(stderr, "open file %s failed err[%d: %s]\n",
                file.c_str(),
                errno,
                strerror(errno));
        return false;
    }

    ssize_t write_len = write(fout, data_buffer.c_str(), data_buffer.size());
    if (write_len == -1) {
        fprintf(stderr, "write file %s failed err[%d: %s]\n",
                file.c_str(),
                errno,
                strerror(errno));
        close(fout);
        return false;
    }
    close(fout);
    return true;
}

bool ParseFromFile(const std::string& file, google::protobuf::Message* pb) {
    if (pb == NULL) {
        return false; 
    }

    char tmp_buffer[PB_BUF_SIZE];
    int fin = open(file.c_str(), OPEN_READ_FLAGS);
    if (fin == -1) {
        fprintf(stderr, "open file %s failed err[%d: %s]\n", 
                file.c_str(),
                errno,
                strerror(errno));
        return false; 
    }

    ssize_t read_len = read(fin, tmp_buffer, PB_BUF_SIZE - 1);
    if (read_len == -1) {
        fprintf(stderr, "read file %s failed err[%d: %s]\n",
                file.c_str(),
                errno,
                strerror(errno)); 
        close(fin);
        return false;
    }
    std::string data_buffer;
    bool ret = true;
    while (read_len > 0) {
        tmp_buffer[read_len] = '\0';
        data_buffer.append(tmp_buffer, read_len); 
        read_len = read(fin, tmp_buffer, PB_BUF_SIZE - 1);
        if (read_len == -1) {
            fprintf(stderr, "read file %s failed err[%d: %s]\n",
                    file.c_str(),
                    errno,
                    strerror(errno));     
            ret = false;
            break;
        }
    }
    close(fin);
    if (!ret) {
        return ret; 
    }

    if (!pb->ParseFromString(data_buffer)) {
        fprintf(stderr, "parse message failed\n"); 
        return false;
    }
    return true;
}

}   // ending namespace test
}   // ending namespace galaxy

/* vim: set ts=4 sw=4 sts=4 tw=100 */
