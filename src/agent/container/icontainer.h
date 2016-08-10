// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once
#include "util/error_code.h"
#include "container_property.h"
#include "boost/shared_ptr.hpp"
#include "protocol/galaxy.pb.h"
#include "protocol/agent.pb.h"
#include <string>

namespace baidu {
namespace galaxy {
namespace container {
class ContainerId {
public:
    ContainerId() {}
    ContainerId(const std::string& group_id, const std::string& container_id) :
        group_id_(group_id),
        container_id_(container_id) {
    }

    bool Empty() {
        return (container_id_.empty());
    }

    ContainerId& SetGroupId(const std::string& id) {
        group_id_ = id;
        return *this;
    }

    ContainerId& SetSubId(const std::string& id) {
        container_id_ = id;
        return *this;
    }

    const std::string& GroupId() const {
        return group_id_;
    }

    const std::string& SubId() const {
        return container_id_;
    }

    const std::string CompactId() const {
        return group_id_ + "_" + container_id_;
    }

    const std::string ToString() const {
        std::string ret;
        ret += "group_id: ";
        ret += group_id_;
        ret += " ";
        ret += "container_id: ";
        ret += container_id_;
        return ret;
    }

    friend bool operator < (const ContainerId& l, const ContainerId& r) {
        //if (l.group_id_ == r.GroupId()) {
        //    return (l.container_id_ < r.container_id_);
        //} else {
        //    return l.group_id_ < r.group_id_;
        //}
        return l.container_id_ < r.container_id_;
    }

    friend bool operator == (const ContainerId& l, const ContainerId& r) {
        //return (l.group_id_ == r.group_id_ && l.container_id_ == r.container_id_);
        return (l.container_id_ == r.container_id_);
    }
private:
    std::string group_id_;
    std::string container_id_;

};

class IContainer {
public:
    IContainer(const ContainerId& id,
            const baidu::galaxy::proto::ContainerDescription& desc);

    virtual ~IContainer();

    static boost::shared_ptr<IContainer> NewContainer(const ContainerId& id,
            const baidu::galaxy::proto::ContainerDescription& desc);
    const ContainerId& Id() const;
    const baidu::galaxy::proto::ContainerDescription& Description() const;
    void  SetDependentVolums(std::map<std::string, std::string>& dvs) {
        dependent_volums_ = dvs;
    }

    virtual baidu::galaxy::util::ErrorCode Construct() = 0;
    virtual baidu::galaxy::util::ErrorCode Destroy() = 0;
    virtual baidu::galaxy::util::ErrorCode Reload(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta) = 0;
    virtual void KeepAlive() = 0;

    virtual boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> ContainerMeta() = 0;
    virtual boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ContainerInfo(bool full_info) = 0;
    virtual boost::shared_ptr<baidu::galaxy::proto::ContainerMetrix> ContainerMetrix() = 0;
    virtual boost::shared_ptr<ContainerProperty> Property() = 0;
    virtual std::string ContainerGcPath() = 0;
protected:
    ContainerId id_;
    const baidu::galaxy::proto::ContainerDescription desc_;
    std::map<std::string, std::string> dependent_volums_;
};

}
}
}
