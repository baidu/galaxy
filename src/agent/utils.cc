// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "common/logging.h"

namespace galaxy {

bool GetDirFilesByPrefix(const std::string& dir, const std::string& prefix, std::vector<std::string>* files) {
    DIR* dir_desc = opendir(dir.c_str());    
    if (dir_desc == NULL) {
        LOG(WARNING, "open dir failed err[%d: %s]",
                errno,
                strerror(errno)); 
        return false;
    }

    struct dirent* dir_entity;
    while ((dir_entity = readdir(dir_desc)) != NULL) {
        std::string log_file_name(dir_entity->d_name);
        if (!prefix.empty() 
                && !boost::starts_with(log_file_name,
                    prefix)) {
            continue; 
        }
        files->push_back(log_file_name);
    }
    return true;
}

static const int WRITE_OPEN_FLAGS = O_WRONLY | O_CREAT;
static const int WRITE_OPEN_MODE = S_IRWXU | S_IRWXG | S_IRWXO;

bool PersistencePbMessage(const std::string& file_name, const google::protobuf::Message& message) {
    int fd = ::open(
            file_name.c_str(), 
            WRITE_OPEN_FLAGS, 
            WRITE_OPEN_MODE);    
    if (fd == -1) {
        LOG(WARNING, "open file failed %s err[%d: %s]",
                file_name.c_str(),
                errno,
                strerror(errno));  
        return false;
    }

    std::string data_buffer;
    message.SerializeToString(&data_buffer);
    size_t size = data_buffer.size();
    int len = ::write(fd, (void*)(&size), sizeof(size));
    if (len == -1) {
        LOG(WARNING, "write pb size failed err[%d: %s]", 
                errno,
                strerror(errno));     
        close(fd);
        return false;
    }
    len = 0;
    len = ::write(fd, data_buffer.c_str(), size);
    if (len == -1) {
        LOG(WARNING, "write pb content failed err[%d: %s]",
                errno,
                strerror(errno)); 
        close(fd);
        return false;
    }
    
    if (0 != fsync(fd)) {
        LOG(WARNING, "fsync failed err[%d: %s]", 
                errno,
                strerror(errno));
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

static const int READ_OPEN_FLAGS = O_RDONLY;

bool ReloadPbMessage(const std::string& file_name, google::protobuf::Message* message) { 
    if (message == NULL) {
        return false; 
    }
    int fd = ::open(
            file_name.c_str(), READ_OPEN_FLAGS);
    if (fd == -1) {
        LOG(WARNING, "open file failed %s err[%d: %s]",
                file_name.c_str(),
                errno,
                strerror(errno)); 
        return false;
    }

    size_t size = 0;
    int len = ::read(fd, (void*)&size, sizeof(size));
    if (len == -1) {
        LOG(WARNING, "read pb size failed err[%d: %s]",
                errno,
                strerror(errno)); 
        close(fd);
        return false;
    }
    len = 0;
    char* tmp_buffer = new char[size];
    len = ::read(fd, tmp_buffer, size);
    if (len == -1) {
        LOG(WARNING, "read pb content failed err[%d: %s]",
                errno,
                strerror(errno)); 
        close(fd);
        delete tmp_buffer;
        return false;
    }
    close(fd);
    std::string data_buffer(tmp_buffer, size);
    delete tmp_buffer;
    if (!message->ParseFromString(data_buffer)) {
        LOG(WARNING, "parse pb from content failed");
        return false; 
    }
    return true;
}

}   // ending namespace galaxy

/* vim: set ts=4 sw=4 sts=4 tw=100 */
