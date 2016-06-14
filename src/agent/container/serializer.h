// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "util/error_code.h"

#include <string>
#include <vector>

namespace baidu {
namespace galaxy {

namespace proto {
    class ContainerMeta;
}

namespace file {
    class DictFile;
}

namespace container {
class Serializer {
public:
    Serializer();
    ~Serializer();
    static std::string WorkKey(const std::string& group_id,
                const std::string& container_id);
    static std::string GcKey(int64_t t,
                const std::string& group_id, 
                const std::string& container_id);

    baidu::galaxy::util::ErrorCode Setup(const std::string& path);
    baidu::galaxy::util::ErrorCode SerializeWork(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta);
    baidu::galaxy::util::ErrorCode SerializeGc(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta);
    baidu::galaxy::util::ErrorCode DeleteWork(const std::string& group_id, const std::string& container_id);
    
    baidu::galaxy::util::ErrorCode DeleteGc(int64_t t, 
                const std::string& group_id, 
                const std::string& container_id);
    baidu::galaxy::util::ErrorCode LoadWork(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> >& metas);
    baidu::galaxy::util::ErrorCode LoadGc(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> >& gc_metas);

    // if key donot exist, return ok, meta.get() is null
    // if key exist return ok, meta.get() is not null
    // if an error occure return not ok
    //
    // meta must be not null as input parameter
    baidu::galaxy::util::ErrorCode Read(const std::string& key,
                boost::shared_ptr<baidu::galaxy::proto::ContainerMeta>& meta);
    baidu::galaxy::util::ErrorCode Delete(const std::string& key);
private:
    boost::scoped_ptr<baidu::galaxy::file::DictFile> dictfile_;
};

}
}
}
