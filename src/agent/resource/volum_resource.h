// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "protocol/galaxy.pb.h"
#include "util/error_code.h"

#include <map>
#include <string>
#include <vector>

namespace baidu {
namespace galaxy {
namespace resource {

class VolumResource {
public:
    class Volum {
    public:
        Volum() :
            total_(0),
            assigned_(0),
            medium_(baidu::galaxy::proto::kDisk) {}

        int64_t total_;
        int64_t assigned_;
        std::string filesystem_;
        std::string mount_point_;
        baidu::galaxy::proto::VolumMedium medium_;
    };

public:
    VolumResource();
    ~VolumResource();
    int Load();

    baidu::galaxy::util::ErrorCode Allocat(const baidu::galaxy::proto::VolumRequired& require);
    baidu::galaxy::util::ErrorCode Release(const baidu::galaxy::proto::VolumRequired& require);
    void Resource(std::map<std::string, Volum>& r);

private:
    baidu::galaxy::util::ErrorCode LoadVolum(const std::string& config, Volum& volum);
    std::map<std::string, Volum> resource_;

};
}
}
}
