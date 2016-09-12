// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#define TEST_CONTAINER_PROPERTY_ON
#ifdef TEST_CONTAINER_PROPERTY_ON
#include "agent/container/container_property.h"

namespace baidu {
namespace galaxy {
namespace test {
class TestContainerProperty : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

TEST_F(TestContainerProperty, Save) {
    baidu::galaxy::container::ContainerProperty cp;
    cp.container_id_ = "container_id";
    cp.group_id_ = "group_id";
    cp.pid_ = 12345;
    cp.workspace_volum_.container_abs_path = "/home/disk1/container/galaxy/1d/";
    cp.workspace_volum_.container_rel_path = "/home/disk11";
    cp.workspace_volum_.medium = "kTmpfs";
    cp.workspace_volum_.phy_gc_path = "/homd";
    cp.workspace_volum_.phy_source_path  = "/home/d";
    cp.workspace_volum_.quota = 10240000;

    {
        baidu::galaxy::container::ContainerProperty::Volum v;
        v.container_abs_path = "/home/disk1/container/galaxy/1d/";
        v.container_rel_path = "/home/disk11";
        v.medium = "kTmpfs";
        v.phy_gc_path = "/homd";
        v.phy_source_path  = "/home/d";
        v.quota = 10240000;
        cp.data_volums_.push_back(v);
    }
    {
        baidu::galaxy::container::ContainerProperty::Volum v;
        v.container_abs_path = "/home/disk1/container/galaxy/1d/";
        v.container_rel_path = "/home/disk11";
        v.medium = "kTmpfs";
        v.phy_gc_path = "/homd";
        v.phy_source_path  = "/home/d";
        v.quota = 10240000;
        cp.data_volums_.push_back(v);
    }

    std::string str = cp.ToString();
    std::cerr << str << std::endl;
}
}
}
}

#endif
