// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_VOLUM_GROUP_ON

#include "protocol/galaxy.pb.h"
#include "agent/volum/tmpfs_volum.h"
#include "agent/volum/symlink_volum.h"
#include "agent/volum/tmpfs_volum.h"
#include "agent/volum/volum_group.h"
#include "agent/volum/volum.h"
#include "agent/util/path_tree.h"

class TestVolumGroup : public testing::Test {
protected:
    static void SetUpTestCase() {
        baidu::galaxy::path::SetRootPath("/tmp/galaxy");
    }

    static void TearDownTestCase() {
    }
};

TEST_F(TestVolumGroup, Construct)
{
    baidu::galaxy::volum::VolumGroup vg;
    baidu::galaxy::proto::VolumRequired vr;
    vr.set_source_path("/home/disk1");
    vr.set_dest_path("/home/disk10");
    vr.set_size(1000000);
    vr.set_medium(baidu::galaxy::proto::kDisk);
    vr.set_use_symlink(true);
    vg.SetWorkspaceVolum(vr);
    vr.set_source_path("/home/disk2");
    vr.set_dest_path("/home/disk20");
    vg.AddDataVolum(vr);
    vr.set_medium(baidu::galaxy::proto::kTmpfs);
    vr.set_dest_path("/home/tmpfs1");
    vr.set_size(20000);
    vg.AddDataVolum(vr);
    vg.SetGcIndex(12345);
    vg.SetContainerId("container_test1");
    EXPECT_EQ(0, vg.Construct().Code());
}

#endif
