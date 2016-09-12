// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"
#include "util/util.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace baidu {
namespace galaxy {
namespace proto {
class VolumRequired;
}

namespace volum {

class Volum {
public:
    Volum();
    virtual ~Volum();

    std::string SourceRootPath();
    std::string SourcePath();
    std::string TargetPath();

    std::string SourceGcRootPath();
    std::string SourceGcPath();
    std::string TargetGcPath();

    void SetGcIndex(int32_t index);  // timestamp is ok
    void SetContainerId(const std::string& container_id);
    void SetUser(const std::string& user) {
        user_ = user;
    }

    const std::string Owner() const {
        return user_;
    }

    void SetDescription(const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr);

    const std::string ContainerId() const;
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> Description() const;

    static boost::shared_ptr<Volum> CreateVolum(const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr);

    virtual baidu::galaxy::util::ErrorCode Destroy();
    virtual baidu::galaxy::util::ErrorCode Construct() = 0;
    virtual baidu::galaxy::util::ErrorCode Gc() {
        return ERRORCODE_OK;
    }
    virtual int64_t Used() = 0;
    virtual std::string ToString() = 0;

private:
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr_;
    std::string container_id_;
    int32_t gc_index_;
    std::string user_;

};

}
}
}
