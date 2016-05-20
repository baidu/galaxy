// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_CGROUP_NETCLS_ON

#include "agent/cgroup/cgroup.h"
#include "agent/cgroup/netcls_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <gflags/gflags.h>

DECLARE_string(cgroup_root_path);

namespace baidu {
namespace galaxy {
namespace test {

class TestNetclsSubsystem : public testing::Test {
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

TEST_F(TestNetclsSubsystem, Name) {
    baidu::galaxy::cgroup::NetclsSubsystem m;
    EXPECT_STREQ(m.Name().c_str(), "net_cls");
}

TEST_F(TestNetclsSubsystem, Path) {
    FLAGS_cgroup_root_path = "/cgroup";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup");
    baidu::galaxy::cgroup::NetclsSubsystem subsystem;
    subsystem.SetContainerId("container");
    subsystem.SetCgroup(cgroup);
    EXPECT_STREQ(subsystem.Path().c_str(), "/cgroup/net_cls/galaxy/container_cgroup");
}

TEST_F(TestNetclsSubsystem, Construct) {
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        cgroup->set_id("cgroup_id1");
        baidu::galaxy::proto::PortRequired* pr = cgroup->add_ports();
        pr->set_port("1234");
        pr->set_real_port("1234");
        pr->set_port_name("port1");
        pr = cgroup->add_ports();
        pr->set_port("1236");
        pr->set_real_port("1236");
        pr->set_port_name("port2");
        pr = cgroup->add_ports();
        pr->set_port("1235");
        pr->set_real_port("1235");
        pr->set_port_name("port3");
        baidu::galaxy::cgroup::NetclsSubsystem ss;
        ss.SetContainerId("container_id");
        ss.SetCgroup(cgroup);
        int min_port;
        int max_port;
        ss.PortRange(&min_port, &max_port);
        EXPECT_EQ(min_port, 1234);
        EXPECT_EQ(max_port, 1236);
        EXPECT_EQ(0, ss.Construct());
        EXPECT_EQ(0, ss.Construct());
        //EXPECT_EQ(0, ss.Destroy());
        //EXPECT_EQ(0, ss.Destroy());
    }
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        cgroup->set_id("cgroup_id2");
        baidu::galaxy::proto::PortRequired* pr = cgroup->add_ports();
        pr->set_port("1234");
        pr->set_real_port("1234");
        pr->set_port_name("port1");
        baidu::galaxy::cgroup::NetclsSubsystem ss;
        ss.SetContainerId("container_id");
        ss.SetCgroup(cgroup);
        int min_port;
        int max_port;
        ss.PortRange(&min_port, &max_port);
        EXPECT_EQ(min_port, 1234);
        EXPECT_EQ(max_port, 1234);
        EXPECT_EQ(0, ss.Construct());
        EXPECT_EQ(0, ss.Construct());
        EXPECT_EQ(0, ss.Destroy());
        EXPECT_EQ(0, ss.Destroy());
    }
}

}
}
}

#endif
