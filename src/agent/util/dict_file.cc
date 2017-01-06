// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dict_file.h"

namespace baidu {
namespace galaxy {
namespace file {

DictFile::DictFile(const std::string& path) :
    path_(path),
    db_(NULL),
    last_ec_(ERRORCODE_OK) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, path, &db_);

    if (!status.ok()) {
        last_ec_ = ERRORCODE(-1, "open db(%s) failed: %s",
                   path_.c_str(),
                   status.ToString().c_str());
    }
}

DictFile::~DictFile() {
    if (NULL != db_) {
        delete db_;
        db_ = NULL;
    }
}

bool DictFile::IsOpen() {
    return NULL != db_;
}

baidu::galaxy::util::ErrorCode DictFile::GetLastError() {
    return last_ec_;
}

baidu::galaxy::util::ErrorCode DictFile::Write(const std::string& key, const std::string& value) {
    assert(NULL != db_);
    leveldb::WriteOptions ops;
    ops.sync = true;
    leveldb::Status status = db_->Put(ops, key, value);

    if (!status.ok()) {
        return ERRORCODE(-1, "persist %s failed: %s",
                key.c_str(),
                status.ToString().c_str());
    }

    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode DictFile::Delete(const std::string& key) {
    assert(NULL != db_);

    leveldb::WriteOptions ops;
    ops.sync = true;
    leveldb::Status status = db_->Delete(ops, key);
    if (!status.ok()) {
        return ERRORCODE(-1, "%s", status.ToString().c_str());
    }
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode DictFile::Scan(const std::string& begin_key, 
            const std::string& end_key, 
            std::vector<Kv>& v) {
    assert(NULL != db_);
    leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
    it->Seek(begin_key);

    while (it->Valid()) {
        DictFile::Kv kv;
        kv.key = it->key().ToString();

        if (kv.key > end_key) {
            break;
        }

        kv.value = it->value().ToString();
        v.push_back(kv);
        it->Next();
    }

    delete it;
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode DictFile::Read(const std::string& key, std::string& value) {
    leveldb::ReadOptions ops;
    leveldb::Status status = db_->Get(ops, key, &value);
    if (status.IsNotFound()) {
        return ERRORCODE(kNotFound, "not exist");
    } else if (!status.ok()) {
        return ERRORCODE(kError, "read %s failed: ", status.ToString().c_str());
    }
    return ERRORCODE(kOk, "ok");
}

}
}
}
