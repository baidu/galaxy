// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#ifdef TEST_CGROUP_CPU_ON
#include "agent/cgroup/cgroup.h"
#include "agent/cgroup/cpu_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <gflags/gflags.h>

DECLARE_string(cgroup_root_path);

namespace baidu {
namespace galaxy {
namespace test {
class TestCpuSubsystem : public testing::Test {
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

TEST_F(TestCpuSubsystem, Name) {
    baidu::galaxy::cgroup::CpuSubsystem cpu;
    EXPECT_STREQ(cpu.Name().c_str(), "cpu");
}

TEST_F(TestCpuSubsystem, Path) {
    FLAGS_cgroup_root_path = "/cgroup";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup");
    baidu::galaxy::cgroup::CpuSubsystem cpu;
    cpu.SetContainerId("container");
    cpu.SetCgroup(cgroup);
    EXPECT_STREQ(cpu.Path().c_str(), "/cgroup/cpu/galaxy/container_cgroup");
}

TEST_F(TestCpuSubsystem, Construct_Destroy) {
    // test hardlimit
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        baidu::galaxy::proto::CpuRequired* cr = new baidu::galaxy::proto::CpuRequired();
        cr->set_excess(false);
        cr->set_milli_core(9000);
        cgroup->set_id("cgroup_hard1");
        cgroup->set_allocated_cpu(cr);
        baidu::galaxy::cgroup::CpuSubsystem cpu;
        cpu.SetContainerId("container1");
        cpu.SetCgroup(cgroup);
        // create multi times, works ok
        EXPECT_EQ(0, cpu.Construct());
        EXPECT_EQ(0, cpu.Construct());
        // destroy multi times, works ok
        EXPECT_EQ(0, cpu.Destroy());
        EXPECT_EQ(0, cpu.Destroy());
    }
    // test softlimit
    {
        FLAGS_cgroup_root_path = "./";
        baidu::galaxy::proto::CpuRequired* cr = new baidu::galaxy::proto::CpuRequired();
        cr->set_excess(true);
        cr->set_milli_core(4000);
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        cgroup->set_id("cgroup_soft1");
        cgroup->set_allocated_cpu(cr);
        baidu::galaxy::cgroup::CpuSubsystem cpu;
        cpu.SetContainerId("container1");
        cpu.SetCgroup(cgroup);
        EXPECT_EQ(0, cpu.Construct());
        EXPECT_EQ(0, cpu.Construct());
        EXPECT_EQ(0, cpu.Destroy());
        EXPECT_EQ(0, cpu.Destroy());
    }
}

TEST_F(TestCpuSubsystem, Attach) {
    FLAGS_cgroup_root_path = "./";
    baidu::galaxy::proto::CpuRequired* cr = new baidu::galaxy::proto::CpuRequired();
    cr->set_excess(true);
    cr->set_milli_core(4000);
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup_soft1");
    cgroup->set_allocated_cpu(cr);
    baidu::galaxy::cgroup::CpuSubsystem cpu;
    cpu.SetContainerId("container1");
    cpu.SetCgroup(cgroup);
    EXPECT_EQ(0, cpu.Construct());
    std::string cmd = "touch " + cpu.Path() + "/cgroup.procs";
    system(cmd.c_str());
    EXPECT_EQ(0, cpu.Attach(12345));
    EXPECT_EQ(0, cpu.Attach(12346));
    std::vector<int> v;
    EXPECT_EQ(0, cpu.GetProcs(v));
    EXPECT_EQ(2u, v.size());
}

}
}
}

#endif
