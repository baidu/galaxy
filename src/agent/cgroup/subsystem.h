// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "util/error_code.h"
#include "util/util.h"

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include <sys/types.h>

#include <string>
#include <map>

namespace baidu {
namespace galaxy {
namespace proto {
class Cgroup;
class CgroupMetrix;
}

namespace cgroup {

class AutoValue {
public:
    typedef enum {
        VT_STRING = 1,
        VT_INT64 = 2,
        VT_BOOLEAN = 3,
        VT_DOUBLE = 4
    } Type;

    Type GetType() const {
        return type_;
    }

    void Set(const std::string& value) {
        str_value_ = value;
        type_ = VT_STRING;
    }

    void Set(const int64_t value) {
        value_.int64_value = value;
        type_ = VT_INT64;
    }

    void Set(const double value) {
        value_.double_value = value;
        type_ = VT_DOUBLE;
    }

    void Set(const bool value) {
        value_.bool_value = value;
    }

    std::string AsString() {
        assert(VT_STRING == type_);
        return str_value_;
    }

    int64_t AsInt64() {
        assert(VT_INT64 == type_);
        return value_.int64_value;
    }

    bool AsBoolean() {
        assert(VT_BOOLEAN == type_);
        return value_.bool_value;
    }

    double AsDouble() {
        assert(VT_DOUBLE == type_);
        return value_.double_value;
    }

private:

    std::string str_value_;
    union Value {
        int64_t int64_value;
        double double_value;
        bool bool_value;
    };
    Value value_;
    Type type_;
};

class Subsystem {
public:
    Subsystem() {}
    virtual ~Subsystem() {}

    static std::string RootPath(const std::string& name);

    Subsystem* SetContainerId(const std::string& container_id) {
        container_id_ = container_id;
        return this;
    }

    Subsystem* SetCgroup(boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup) {
        cgroup_ = cgroup;
        return this;
    }

    virtual boost::shared_ptr<Subsystem> Clone() = 0;
    virtual std::string Name() = 0;
    virtual baidu::galaxy::util::ErrorCode Construct() = 0;

    virtual baidu::galaxy::util::ErrorCode Destroy();
    virtual void Kill();
    virtual baidu::galaxy::util::ErrorCode Attach(pid_t pid);
    virtual baidu::galaxy::util::ErrorCode GetProcs(std::vector<int>& pids);
    virtual std::string Path();

    virtual baidu::galaxy::util::ErrorCode Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
        return ERRORCODE_OK;
    }
    //virtual bool Empty();

protected:
    std::string container_id_;
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup_;

};

baidu::galaxy::util::ErrorCode Attach(const std::string& file, int64_t value, bool append = false);
baidu::galaxy::util::ErrorCode Attach(const std::string& file, const std::string& value, bool append = false);


int64_t CfsToMilliCore(int64_t cfs);
int64_t ShareToMilliCore(int64_t share);
int64_t MilliCoreToCfs(int64_t millicore);
int64_t MilliCoreToShare(int64_t millicore);

} //namespace cgroup
} //namespace galaxy
} //namespace baidu
