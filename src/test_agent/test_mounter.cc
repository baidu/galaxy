// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#ifdef TEST_MOUNTER_ON

#include "agent/volum/mounter.h"

TEST(TestMounter, ListMounters) {
    std::map<std::string, boost::shared_ptr<baidu::galaxy::volum::Mounter> > mounters;
    EXPECT_EQ(0, baidu::galaxy::volum::ListMounters(mounters).Code());
    std::map<std::string, boost::shared_ptr<baidu::galaxy::volum::Mounter> >::iterator iter = mounters.begin();

    while (iter != mounters.end()) {
        std::cerr << iter->second->ToString() << std::endl;
        iter++;
    }
}
#endif
