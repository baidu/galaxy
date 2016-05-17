// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#include "agent/volum/symlink_volum.h"
#include "agent/volum/volum.h"
#include "protocol/galaxy.pb.h"
#include "agent/util/path_tree.h"

#ifdef TEST_SYMLINK_VOLUM_ON

namespace baidu {
namespace galaxy {
namespace test {

class TestSymlinkVolum : public testing::Test {
protected:
    static void SetUpTestCase() {
        baidu::galaxy::path::SetRootPath("/home/galaxy");
    }

    static void TearDownTestCase() {
    }
};

TEST_F(TestSymlinkVolum, SourcePath_TargetPath) {
    baidu::galaxy::volum::SymlinkVolum volum;
    volum.SetContainerId("container_id");
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired);
    vr->set_dest_path("/home/disk10");
    vr->set_source_path("/home/disk1");
    vr->set_exclusive(false);
    vr->set_medium(baidu::galaxy::proto::kDisk);
    vr->set_use_symlink(true);
    vr->set_type(baidu::galaxy::proto::kEmptyDir);
    volum.SetDescription(vr);
    EXPECT_STREQ(volum.SourcePath().c_str(), "/home/disk1/galaxy/container_id/home/disk10");
    EXPECT_STREQ(volum.TargetPath().c_str(), "/home/galaxy/work_dir/container_id/home/disk10");
}

TEST_F(TestSymlinkVolum, Construct) {
    {
        baidu::galaxy::volum::SymlinkVolum volum;
        volum.SetContainerId("container_id");
        boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired);
        vr->set_dest_path("/home/disk10");
        vr->set_source_path("/home/disk1");
        vr->set_exclusive(false);
        vr->set_medium(baidu::galaxy::proto::kDisk);
        vr->set_use_symlink(true);
        vr->set_type(baidu::galaxy::proto::kEmptyDir);
        volum.SetDescription(vr);
        EXPECT_EQ(0, volum.Construct());
    }
    {
        baidu::galaxy::volum::SymlinkVolum volum;
        volum.SetContainerId("container_id");
        boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired);
        vr->set_dest_path("/home/disk10");
        vr->set_source_path("/home/disk2");
        vr->set_exclusive(false);
        vr->set_medium(baidu::galaxy::proto::kDisk);
        vr->set_use_symlink(true);
        vr->set_type(baidu::galaxy::proto::kEmptyDir);
        volum.SetDescription(vr);
        EXPECT_EQ(-1, volum.Construct());
    }
    //    EXPECT_EQ(0, volum.Construct());
    //    EXPECT_EQ(0, volum.Destroy());
    //    EXPECT_EQ(0, volum.Destroy());
}


}
}
}
#endif
