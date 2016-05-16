// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "subsystem.h"


namespace baidu {
namespace galaxy {
namespace cgroup {

class CpuacctSubsystem : public Subsystem {
    public:
        CpuacctSubsystem();
        ~CpuacctSubsystem();
        std::string Name();
        int Construct();
        boost::shared_ptr<Subsystem> Clone();
        boost::shared_ptr<google::protobuf::Message> Status();
};

}
}
}
