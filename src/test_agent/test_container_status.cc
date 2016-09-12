// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_CONTAINER_STATUS_ON
#include "agent/container/container_status.h"
#include "agent/container/container.h"
#include <map>

class TestContainerStatus : public testing::Test {
protected:

    static void SetUpTestCase() {
        baidu::galaxy::container::ContainerStatus::Setup();
    }

    static void TearDownTestCase() {
    }
};

TEST_F(TestContainerStatus, EnterAllocating) {
    baidu::galaxy::container::ContainerStatus cs("container_id");
    {
        cs.SetStatus(baidu::galaxy::proto::kContainerPending);
        baidu::galaxy::util::ErrorCode ec = cs.EnterAllocating();
        EXPECT_EQ(baidu::galaxy::util::kErrorOk, ec.Code()) << ec.Message();
    }
    {
        cs.SetStatus(baidu::galaxy::proto::kContainerAllocating);
        baidu::galaxy::util::ErrorCode ec = cs.EnterAllocating();
        EXPECT_EQ(baidu::galaxy::util::kErrorRepeated, ec.Code()) << ec.Message();
    }
    {
        cs.SetStatus(baidu::galaxy::proto::kContainerReady);
        baidu::galaxy::util::ErrorCode ec = cs.EnterAllocating();
        EXPECT_EQ(baidu::galaxy::util::kErrorNotAllowed, ec.Code()) << ec.Message();
    }
    {
        cs.SetStatus(baidu::galaxy::proto::kContainerError);
        baidu::galaxy::util::ErrorCode ec = cs.EnterAllocating();
        EXPECT_EQ(baidu::galaxy::util::kErrorNotAllowed, ec.Code()) << ec.Message();
    }
    {
        cs.SetStatus(baidu::galaxy::proto::kContainerFinish);
        baidu::galaxy::util::ErrorCode ec = cs.EnterAllocating();
        EXPECT_EQ(baidu::galaxy::util::kErrorNotAllowed, ec.Code()) << ec.Message();
    }
    {
        cs.SetStatus(baidu::galaxy::proto::kContainerDestroying);
        baidu::galaxy::util::ErrorCode ec = cs.EnterAllocating();
        EXPECT_EQ(baidu::galaxy::util::kErrorNotAllowed, ec.Code()) << ec.Message();
    }
}

TEST_F(TestContainerStatus, EnterReady) {
    baidu::galaxy::container::ContainerId id("job_20160616_230218_396_diting",
                "job_20160616_230218_396_diting.pod_14");

    std::map<baidu::galaxy::container::ContainerId, int> m;
    m[id] = 1;
    std::map<baidu::galaxy::container::ContainerId, int>::iterator iter = m.find(id);
    EXPECT_TRUE(iter != m.end());
}

#endif
