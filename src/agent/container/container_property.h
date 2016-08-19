// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"
#include <stdint.h>
#include <vector>
#include <string>


namespace baidu {
namespace galaxy {
namespace container {

class ContainerProperty {
public:
    std::string ToString() const;
public:
    struct Volum {
        std::string container_rel_path;
        std::string phy_source_path;
        std::string container_abs_path;
        std::string phy_gc_path;
        std::string phy_gc_root_path;
        int64_t quota;
        std::string medium;
        std::string ToString() const;
    };

    Volum workspace_volum_;
    std::vector<Volum> data_volums_;
    int pid_;
    int32_t created_time_;
    int32_t destroy_time_;
    std::string group_id_;
    std::string container_id_;

};

}
}
}
