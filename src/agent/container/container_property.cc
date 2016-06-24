// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "container_property.h"
#include "agent/util/output_stream_file.h"

#include <sstream>
#include <iostream>

#define GENPROPERTY(stream, key, value) do { \
    stream << key << " : " << value << "\n"; \
} while(0)

namespace baidu {
namespace galaxy {
namespace container {

//std::string container_rel_path;
//std::string phy_source_path;
//std::string container_abs_path;
//std::string phy_gc_path;
//std::string phy_gc_root_path;
//int64_t size;
//std::string medium;

std::string ContainerProperty::Volum::ToString() const {
    std::stringstream ss;
    GENPROPERTY(ss, "container_rel_path", container_rel_path);
    GENPROPERTY(ss, "container_abs_path", container_abs_path);
    GENPROPERTY(ss, "phy_source_path", phy_source_path);
    GENPROPERTY(ss, "phy_gc_path", phy_gc_path);
    GENPROPERTY(ss, "phy_gc_root_path", phy_gc_root_path);
    GENPROPERTY(ss, "quota", quota);
    GENPROPERTY(ss, "medium", medium);
    return ss.str();
}


std::string ContainerProperty::ToString() const {
    std::stringstream ss;
    GENPROPERTY(ss, "group_id", group_id_);
    GENPROPERTY(ss, "container_id", container_id_);
    GENPROPERTY(ss, "appworker", pid_);
    ss << "\n\n#workspace volum\n";
    ss << workspace_volum_.ToString();
    ss << "\n\n";

    for (size_t i = 0; i < data_volums_.size(); i++) {
        ss << "\n#data volum " << i << "\n";
        ss << data_volums_[i].ToString();
    }
    return ss.str();

   }
}
}
}
