// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_CGROUP_TCPTHROT_ON

#include "agent/cgroup/cgroup.h"
#include "agent/cgroup/tcp_throt_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <gflags/gflags.h>

DECLARE_string(cgroup_root_path);

namespace baidu {
namespace galaxy {
namespace test {

class TestTcpthrotSubsystem : public testing::Test {
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

TEST_F(TestTcpthrotSubsystem, Name) {
    baidu::galaxy::cgroup::TcpThrotSubsystem m;
    EXPECT_STREQ(m.Name().c_str(), "tcp_throt");
}

TEST_F(TestTcpthrotSubsystem, Path) {
    FLAGS_cgroup_root_path = "/cgroup";
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
    cgroup->set_id("cgroup");
    baidu::galaxy::cgroup::TcpThrotSubsystem subsystem;
    subsystem.SetContainerId("container");
    subsystem.SetCgroup(cgroup);
    EXPECT_STREQ(subsystem.Path().c_str(), "/cgroup/tcp_throt/galaxy/container_cgroup");
}

TEST_F(TestTcpthrotSubsystem, Construct) {
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        cgroup->set_id("cgroup_id1");
        baidu::galaxy::proto::TcpthrotRequired* tr = new baidu::galaxy::proto::TcpthrotRequired();
        tr->set_recv_bps_excess(false);
        tr->set_recv_bps_quota(100);
        tr->set_send_bps_excess(true);
        tr->set_send_bps_quota(2000);
        cgroup->set_allocated_tcp_throt(tr);
        baidu::galaxy::cgroup::TcpThrotSubsystem ss;
        ss.SetContainerId("container_id");
        ss.SetCgroup(cgroup);
        EXPECT_EQ(0, ss.Construct());
        EXPECT_EQ(0, ss.Construct());
        //EXPECT_EQ(0, ss.Destroy());
        //EXPECT_EQ(0, ss.Destroy());
    }
    {
        FLAGS_cgroup_root_path = "./";
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup(new baidu::galaxy::proto::Cgroup);
        cgroup->set_id("cgroup_id2");
        baidu::galaxy::proto::TcpthrotRequired* tr = new baidu::galaxy::proto::TcpthrotRequired();
        tr->set_recv_bps_excess(true);
        tr->set_recv_bps_quota(12345);
        tr->set_send_bps_excess(false);
        tr->set_send_bps_quota(54321);
        cgroup->set_allocated_tcp_throt(tr);
        baidu::galaxy::cgroup::TcpThrotSubsystem ss;
        ss.SetContainerId("container_id");
        ss.SetCgroup(cgroup);
        EXPECT_EQ(0, ss.Construct());
        EXPECT_EQ(0, ss.Construct());
        //EXPECT_EQ(0, ss.Destroy());
        //EXPECT_EQ(0, ss.Destroy());
    }
}

}
}
}

#endif
