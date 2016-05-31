// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"
#ifdef TEST_FILE_INPUT_STREAM

#include "agent/util/input_stream_file.h"

class TestInputStreamFile : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

TEST_F(TestInputStreamFile, normal) {
    {
        baidu::galaxy::file::InputStreamFile is("/proc/cpuinfo");
        EXPECT_TRUE(is.IsOpen());
        EXPECT_FALSE(is.Eof());

        while (!is.Eof()) {
            std::string line;
            baidu::galaxy::util::ErrorCode ec = is.ReadLine(line);
            EXPECT_EQ(ec.Code(), 0) << ec.Message();
            //std::cerr << line;
        }
    }
    {
        // run as root or permission denied
        baidu::galaxy::file::InputStreamFile is("/proc/1/environ");
        baidu::galaxy::util::ErrorCode err;
        is.GetLastError(err);

        EXPECT_TRUE(is.IsOpen()) << err.Message();
        EXPECT_FALSE(is.Eof());
        std::string content;

        while (!is.Eof()) {
            char buf[1024];
            size_t size = sizeof buf;
            baidu::galaxy::util::ErrorCode ec = is.Read(buf, size);
            EXPECT_EQ(ec.Code(), 0);
            content.append(buf, size);
        }
    }
}

TEST_F(TestInputStreamFile, exception) {
    {
        baidu::galaxy::file::InputStreamFile is("/proc/not_exist_file");
        EXPECT_FALSE(is.IsOpen());
    }
}
#endif
