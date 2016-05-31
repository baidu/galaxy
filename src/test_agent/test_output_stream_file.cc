// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#ifdef TEST_OUTPUT_STREAM_FILE_ON
#include "agent/util/output_stream_file.h"
class TestOutputStreamFile : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {
        system("rm test_output_stream_file.dat");
    }
};

TEST_F(TestOutputStreamFile, write) {
    {
        baidu::galaxy::file::OutputStreamFile of("test_output_stream_file.dat", "w");
        EXPECT_TRUE(of.IsOpen());
        char data[] = "hello galaxy";
        size_t len = sizeof data;
        baidu::galaxy::util::ErrorCode ec = of.Write(data, len);
        EXPECT_EQ(ec.Code(), 0);
        EXPECT_EQ(len, sizeof data);
    }
}
#endif
