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

std::string Serializer::WorkContainerKey(const std::string& group_id,
            const std::string& container_id) {
    return "#_" + group_id + "_" + container_id;
}

std::string GcContainerKey(const std::string& group_id, 
            const std::string& container_id) {
    return "!_" + group_id + "_" + container_id;
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
    std::string key = Serializer::WorkContainerKey(meta->group_id(),meta->container_id());

    baidu::galaxy::util::ErrorCode ec = dictfile_->Write(key, meta->SerializeAsString());

    if (ec.Code() != 0) {
        return ERRORCODE(-1, "serialize work container meta failed, key is %s : %s",
                key.c_str(),
                ec.Message().c_str());
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode Serializer::SerializeGc(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta) {
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode Serializer::DeleteWork(const std::string& group_id, const std::string& container_id) {
    const std::string key = Serializer::WorkContainerKey(group_id, container_id);
    baidu::galaxy::util::ErrorCode ec = dictfile_->Delete(key);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, "delete key %s failed: %s",
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


baidu::galaxy::util::ErrorCode Serializer::LoadGc(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> >& gc_metas) {
    return ERRORCODE_OK;
}

}
}
}
