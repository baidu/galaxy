// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_CGROUP_FREEZER_ON

#include "agent/cgroup/cgroup.h"
#include "agent/cgroup/freezer_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <gflags/gflags.h>

DECLARE_string(cgroup_root_path);

namespace baidu {
namespace galaxy {
namespace test {

class TestFreezerSubsystem : public testing::Test {
protected:

    static void SetUpTestCase() {
        FLAGS_cgroup_root_path = "./";
    }

    static void TearDownTestCase() {
        std::string path = FLAGS_cgroup_root_path + "galaxy";
        std::string cmd = "rm -rf " + path;
        //system(cmd.c_str());
    }

};

TEST_F(TestFreezerSubsystem, Name) {
    baidu::galaxy::cgroup::FreezerSubsystem m;
    EXPECT_STREQ(m.Name().c_str(), "freezer");
}

TEST_F(TestFreezerSubsystem, Path) {
    FLAGS_cgroup_root_path = "/cgroup";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup");
    baidu::galaxy::cgroup::FreezerSubsystem subsystem;
    subsystem.SetContainerId("container");
    subsystem.SetCgroup(cgroup);
    EXPECT_STREQ(subsystem.Path().c_str(), "/cgroup/freezer/galaxy/container_cgroup");
}

TEST_F(TestFreezerSubsystem, Construct) {
    FLAGS_cgroup_root_path = "./";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup_id");
    baidu::galaxy::cgroup::FreezerSubsystem ss;
    ss.SetContainerId("container_id");
    ss.SetCgroup(cgroup);
    EXPECT_EQ(0, ss.Construct());
    EXPECT_EQ(0, ss.Construct());
    EXPECT_EQ(0, ss.Destroy());
    EXPECT_EQ(0, ss.Destroy());
}

TEST_F(TestFreezerSubsystem, Freeze) {
    FLAGS_cgroup_root_path = "./";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup_id_hardlimit");
    baidu::galaxy::cgroup::FreezerSubsystem ss;
    ss.SetContainerId("container_id");
    ss.SetCgroup(cgroup);
    EXPECT_EQ(0, ss.Construct());
    EXPECT_EQ(0, ss.Freeze());
    EXPECT_EQ(0, ss.Destroy());
}

TEST_F(TestFreezerSubsystem, Thaw) {
    FLAGS_cgroup_root_path = "./";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup_id");
    baidu::galaxy::cgroup::FreezerSubsystem ss;
    ss.SetContainerId("container_id");
    ss.SetCgroup(cgroup);
    EXPECT_EQ(0, ss.Construct());
    EXPECT_EQ(0, ss.Thaw());
    EXPECT_EQ(0, ss.Destroy());
}

}
}
}

#endif
