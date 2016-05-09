// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
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

    std::string SourcePath();
    std::string TargetPath();

    void SetContainerId(const std::string& container_id);
    const std::string ContainerId() const;
    void SetDescription(const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr);
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> Description() const;

    static boost::shared_ptr<Volum> CreateVolum(const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr);

    virtual int Destroy();
    virtual int Construct() = 0;
    virtual int64_t Used() = 0;
    virtual std::string ToString() = 0;

private:
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr_;
    std::string container_id_;

};

}
}
}
