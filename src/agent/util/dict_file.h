// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"
#include "leveldb/db.h"

#include <string>
#include <vector>

namespace baidu {
namespace galaxy {
namespace file {

static const int kNotFound = 1;
static const int kError = -1;
static const int kOk = 0;

class DictFile {
public:
    class Kv {
    public:
        std::string key;
        std::string value;
    };

    explicit DictFile(const std::string& path);
    ~DictFile();
    bool IsOpen();
    baidu::galaxy::util::ErrorCode GetLastError();

    baidu::galaxy::util::ErrorCode Write(const std::string& key, const std::string& value);

    // if not exist return kNotFound, value is empty
    baidu::galaxy::util::ErrorCode Read(const std::string& key, std::string& value);
    baidu::galaxy::util::ErrorCode Delete(const std::string& key);
    baidu::galaxy::util::ErrorCode Scan(const std::string& begin_key, 
                const std::string& end_key,
                std::vector<Kv>& v);

private:
    const std::string path_;
    leveldb::DB* db_;
    baidu::galaxy::util::ErrorCode last_ec_;

};

}
}
}
