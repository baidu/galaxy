// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "serializer.h"
#include "agent/util/dict_file.h"
#include "boost/smart_ptr/shared_ptr.hpp"
#include "protocol/galaxy.pb.h"

#include "util/dict_file.h"


namespace baidu {
namespace galaxy {
namespace container {
Serializer::Serializer() {
}
Serializer::~Serializer() {
}

std::string Serializer::WorkKey(const std::string& group_id,
            const std::string& container_id) {
    return "#_" + group_id + "_" + container_id;
}

std::string Serializer::GcKey(int64_t gc_time, 
            const std::string& group_id, 
            const std::string& container_id) {
    char buf[32] = {0};
    snprintf(buf, sizeof buf, "%012lld", (long long int )gc_time);
    std::string str_t(buf);
    return "!_" + str_t + "_" + group_id + "_" + container_id;
}


baidu::galaxy::util::ErrorCode Serializer::Setup(const std::string& path) {
    assert(NULL == dictfile_.get());
    dictfile_.reset(new baidu::galaxy::file::DictFile(path));

    if (!dictfile_->IsOpen()) {
        return ERRORCODE(-1, "setup serializer failed: %s",
                dictfile_->GetLastError().Message().c_str());
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode Serializer::SerializeWork(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta) {
    assert(dictfile_->IsOpen());
    assert(NULL != meta.get());
    std::string key = Serializer::WorkKey(meta->group_id(),meta->container_id());

    baidu::galaxy::util::ErrorCode ec = dictfile_->Write(key, meta->SerializeAsString());

    if (ec.Code() != 0) {
        return ERRORCODE(-1, "serialize work container meta failed, key is %s : %s",
                key.c_str(),
                ec.Message().c_str());
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode Serializer::SerializeGc(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta) {
    assert(dictfile_->IsOpen());
    assert(NULL != meta.get());
    std::string key = Serializer::GcKey(meta->destroy_time(), meta->group_id(), meta->container_id());
    baidu::galaxy::util::ErrorCode ec = dictfile_->Write(key, meta->SerializeAsString());

    if (ec.Code() != 0) {
        return ERRORCODE(-1, "serialize gc container meta failed, key is %s: %s",
                    key.c_str(),
                    ec.Message().c_str());
    }
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode Serializer::DeleteWork(const std::string& group_id,
            const std::string& container_id) {
    assert(dictfile_->IsOpen());
    const std::string key = Serializer::WorkKey(group_id, container_id);
    baidu::galaxy::util::ErrorCode ec = dictfile_->Delete(key);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, "delete work key %s failed: %s",
                    key.c_str(),
                    ec.Message().c_str());
    }
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode Serializer::Delete(const std::string& key) {
    assert(dictfile_->IsOpen());
    baidu::galaxy::util::ErrorCode ec = dictfile_->Delete(key);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, ec.Message().c_str());
    }
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode Serializer::DeleteGc(int64_t time, 
            const std::string& group_id, 
            const std::string& container_id) {

    const std::string key = Serializer::GcKey(time, group_id, container_id);
    baidu::galaxy::util::ErrorCode ec = dictfile_->Delete(key);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, "delete gc key %s failed: %s",
                    key.c_str(),
                    ec.Message().c_str());
    }
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode Serializer::LoadWork(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> >& metas) {
    assert(dictfile_->IsOpen());
    std::string begin_key = "#_!";
    std::string end_key = "#_~";

    std::vector<baidu::galaxy::file::DictFile::Kv> v;
    baidu::galaxy::util::ErrorCode ec = dictfile_->Scan(begin_key, end_key, v);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, 
                    "scan work-contaner meta failed: %s", 
                    ec.Message().c_str());
    }

    for (size_t i = 0; i < v.size(); i++) {
        boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> cm(new baidu::galaxy::proto::ContainerMeta);
        cm->ParseFromString(v[i].value);
        metas.push_back(cm);
    }
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode Serializer::LoadGc(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> >& metas) {
    assert(dictfile_->IsOpen());
    std::string begin_key = "!_!";
    std::string end_key = "!_~";

    std::vector<baidu::galaxy::file::DictFile::Kv> v;
    baidu::galaxy::util::ErrorCode ec = dictfile_->Scan(begin_key, end_key, v);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, 
                    "scan gc-contaner meta failed: %s", 
                    ec.Message().c_str());
    }

    for (size_t i = 0; i < v.size(); i++) {
        boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> cm(new baidu::galaxy::proto::ContainerMeta);
        cm->ParseFromString(v[i].value);
        metas.push_back(cm);
    }
    return ERRORCODE_OK;
}

 baidu::galaxy::util::ErrorCode Serializer::Read(const std::string& key,
                boost::shared_ptr<baidu::galaxy::proto::ContainerMeta>& meta) {
     assert(NULL != meta.get());
     std::string value;
     baidu::galaxy::util::ErrorCode ec = dictfile_->Read(key, value);
     if (baidu::galaxy::file::kOk == 0) {
         if (!meta->ParseFromString(value)) {
             meta.reset();
             return ERRORCODE(-1, "parse failed: %d", value.size());
         }
     } else if (baidu::galaxy::file::kNotFound == ec.Code()) {
         meta.reset();
         return ERRORCODE(0, ec.Message().c_str());
     } else {
         meta.reset();
         return ERRORCODE(-1, ec.Message().c_str());
     }

     return ERRORCODE_OK;
 }


}
}
}
