// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#include "protocol/galaxy.pb.h"
#include "agent/container/container.h"

#include "agent/util/path_tree.h"
#include "agent/cgroup/subsystem_factory.h"

#include <gflags/gflags.h>

#ifdef TEST_CONTAINER_ON

DECLARE_string(cgroup_root_path);
DECLARE_string(mount_templat);

namespace baidu {
namespace galaxy {
namespace test {

class TestContainer : public testing::Test {
protected:

    static void SetUpTestCase() {
        baidu::galaxy::container::ContainerStatus::Setup();
        FLAGS_cgroup_root_path = "/tmp/cgroup";
        FLAGS_mount_templat = "/home,/bin,/boot,/cgroups,/dev,/etc,/lib,/lib64,/lost+found,/media,/misc,/mnt,/opt,/sbin,/selinux,/srv,/sys,/tmp,/usr,/var";
        baidu::galaxy::path::SetRootPath("/tmp/galaxy_test");
        baidu::galaxy::cgroup::SubsystemFactory::GetInstance()->Setup();
    }

    static void TearDownTestCase() {
        std::string path = FLAGS_cgroup_root_path + "galaxy";
        std::string cmd = "rm -rf " + path;
        //system(cmd.c_str());
    }
};

TEST_F(TestContainer, Construct1)
{
    baidu::galaxy::proto::ContainerDescription desc;
    desc.set_cmd_line("sh -x test.sh");
    desc.set_run_user("galaxy");
    baidu::galaxy::proto::VolumRequired* workspace  = new baidu::galaxy::proto::VolumRequired();
    workspace->set_dest_path("/home/workspace");
    workspace->set_source_path("/home/disk1");
    workspace->set_exclusive(true);
    workspace->set_medium(baidu::galaxy::proto::kDisk);
    workspace->set_use_symlink(true);
    workspace->set_size(1024 * 1024 * 1024);
    desc.set_allocated_workspace_volum(workspace);
    baidu::galaxy::proto::VolumRequired* datavolum1 = desc.add_data_volums();;
    datavolum1->set_dest_path("/home/disk10");
    datavolum1->set_source_path("/home/disk1");
    datavolum1->set_size(1024 * 1024);
    datavolum1->set_medium(baidu::galaxy::proto::kDisk);
    datavolum1->set_use_symlink(true);
    baidu::galaxy::proto::VolumRequired* datavolum2 = desc.add_data_volums();;
    datavolum2->set_dest_path("/home/disk20");
    datavolum2->set_size(1024 * 1024);
    datavolum2->set_medium(baidu::galaxy::proto::kTmpfs);
    baidu::galaxy::proto::Cgroup* cgroup = desc.add_cgroups();
    cgroup->set_id("cgroup1");
    //cpu
    baidu::galaxy::proto::CpuRequired* cr = new baidu::galaxy::proto::CpuRequired();
    cr->set_milli_core(1000);
    cr->set_excess(false);
    cgroup->set_allocated_cpu(cr);
    //memory
    baidu::galaxy::proto::MemoryRequired* mr = new baidu::galaxy::proto::MemoryRequired();
    mr->set_excess(false);
    mr->set_size(1024 * 1024 * 100);
    cgroup->set_allocated_memory(mr);
    // port
    baidu::galaxy::proto::PortRequired* pr = cgroup->add_ports();
    pr->set_port("dynamic");
    pr->set_port_name("port1");
    pr->set_real_port("8000");
    // tcp_throt
    baidu::galaxy::proto::TcpthrotRequired* tr = new baidu::galaxy::proto::TcpthrotRequired();
    tr->set_recv_bps_excess(false);
    tr->set_recv_bps_quota(200000);
    tr->set_send_bps_excess(false);
    tr->set_send_bps_quota(1000000);
    cgroup->set_allocated_tcp_throt(tr);
    baidu::galaxy::container::ContainerId id("container_group_id", "container_id");
    baidu::galaxy::container::Container container(id, desc);
    EXPECT_EQ(0, container.Construct());
    sleep(1);
    int n = 100;
    while (n--) {
        container.KeepAlive();
        sleep(1);
    }
    EXPECT_EQ(0, container.Destroy());
}


}
}
}
#endif
