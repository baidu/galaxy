// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "cgroup.h"
#include <boost/shared_ptr.hpp>

namespace baidu {
namespace galaxy {
namespace cgroup {

class MemorySubsystem : public Cgroup {
public:
    class Option {
    public:
        const std::string Path() const {
            return "";
        }

        int64_t LimitInBytes() const {
            return 0L;
        }

        bool Excess() const {
            return false;
        }
    };
public:
    MemorySubsystem(boost::shared_ptr<Option> op);
    ~MemorySubsystem();
    
    std::string Name();
    int Construct();
    int Attach(pid_t pid);
    boost::shared_ptr<google::protobuf::Message> Status();
    int GetProcs();
    
private:
    boost::shared_ptr<Option> _op;
};

} //namespace cgroup
} //namespace galaxy
} //namespace baidu
