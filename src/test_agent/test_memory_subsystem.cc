// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_CGROUP_MEMORY_ON

#include "agent/cgroup/cgroup.h"
#include "agent/cgroup/memory_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <gflags/gflags.h>

DECLARE_string(cgroup_root_path);

namespace baidu {
namespace galaxy {
namespace test {

class TestMemorySubsystem : public testing::Test {
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

TEST_F(TestMemorySubsystem, Name) {
    baidu::galaxy::cgroup::MemorySubsystem m;
    EXPECT_STREQ(m.Name().c_str(), "memory");
}
TEST_F(TestMemorySubsystem, Path) {
    FLAGS_cgroup_root_path = "/cgroup";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup");
    baidu::galaxy::cgroup::MemorySubsystem m;
    m.SetContainerId("container");
    m.SetCgroup(cgroup);
    EXPECT_STREQ(m.Path().c_str(), "/cgroup/memory/galaxy/container_cgroup");
}

TEST_F(TestMemorySubsystem, Construct) {
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        baidu::galaxy::proto::MemoryRequired* mr = new baidu::galaxy::proto::MemoryRequired();
        mr->set_excess(false);
        mr->set_size(1000);
        cgroup->set_allocated_memory(mr);
        cgroup->set_id("cgroup_id_hardlimit");
        baidu::galaxy::cgroup::MemorySubsystem ms;
        ms.SetContainerId("container_id");
        ms.SetCgroup(cgroup);
        EXPECT_EQ(0, ms.Construct());
        EXPECT_EQ(0, ms.Construct());
        //EXPECT_EQ(0, ms.Destroy());
        //EXPECT_EQ(0, ms.Destroy());
    }
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        baidu::galaxy::proto::MemoryRequired* mr = new baidu::galaxy::proto::MemoryRequired();
        mr->set_excess(true);
        mr->set_size(2000);
        cgroup->set_allocated_memory(mr);
        cgroup->set_id("cgroup_id_soft");
        baidu::galaxy::cgroup::MemorySubsystem ms;
        ms.SetContainerId("container_id");
        ms.SetCgroup(cgroup);
        EXPECT_EQ(0, ms.Construct());
        EXPECT_EQ(0, ms.Construct());
        //EXPECT_EQ(0, ms.Destroy());
        //EXPECT_EQ(0, ms.Destroy());
    }
}
}
}
}

#endif
