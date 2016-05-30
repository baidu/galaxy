// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#ifdef TEST_DICT_FILE_ON
#include "agent/util/dict_file.h"
#include <sstream>

namespace baidu {
namespace galaxy {
namespace test {

class TestDictFile : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {
        system("rm unittest_dict_file -rf");
    }
};

TEST_F(TestDictFile, nornal) {
    baidu::galaxy::file::DictFile df("./unittest_dict_file");
    EXPECT_TRUE(df.IsOpen());

    for (int i = 0; i < 10; i ++) {
        std::stringstream ss1;
        ss1 << i;
        std::stringstream ss2;
        ss2 << i * 10;
        baidu::galaxy::util::ErrorCode ec = df.Write(ss1.str(), ss2.str());
        EXPECT_EQ(ec.Code(), 0) << ec.Message();
    }

    std::stringstream sskey1;
    sskey1 << 3;
    std::stringstream sskey2;
    sskey2 << 8;
    std::vector<baidu::galaxy::file::DictFile::Kv> v;
    baidu::galaxy::util::ErrorCode ec = df.Scan(sskey1.str(), sskey2.str(), v);
    EXPECT_EQ(ec.Code(), 0) << ec.Message();
    EXPECT_EQ(v.size(), (size_t)6);
    EXPECT_STREQ(v[0].key.c_str(), "3");
    EXPECT_STREQ(v[0].value.c_str(), "30");
    EXPECT_STREQ(v[5].key.c_str(), "8");
    EXPECT_STREQ(v[5].value.c_str(), "80");
}
}
}
}


#endif
