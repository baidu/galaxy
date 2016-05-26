// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#include "protocol/galaxy.pb.h"
#include "agent/volum/tmpfs_volum.h"
#include "agent/volum/volum.h"
#include "agent/util/path_tree.h"

#ifdef TEST_TMPFS_VOLUM_ON

class TestTmpfsVolum : public testing::Test {
protected:
    static void SetUpTestCase() {
        baidu::galaxy::path::SetRootPath("/home/galaxy");
    }

    static void TearDownTestCase() {
    }
};

TEST_F(TestTmpfsVolum, Construct)
{
    baidu::galaxy::volum::TmpfsVolum volum;
    volum.SetContainerId("container_id");
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired);
    vr->set_dest_path("/home/tmpfs");
    vr->set_exclusive(false);
    vr->set_medium(baidu::galaxy::proto::kTmpfs);
    vr->set_use_symlink(false);
    vr->set_size(1024 * 1024 * 1024);
    vr->set_type(baidu::galaxy::proto::kEmptyDir);
    volum.SetDescription(vr);
    EXPECT_EQ(0, volum.Construct().Code());
}

#endif
