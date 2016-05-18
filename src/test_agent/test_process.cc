// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_PROCESS_ON
#include "agent/container/process.h"
#include "boost/bind.hpp"

#include <unistd.h>
#include <sys/types.h>

namespace baidu {
namespace galaxy {
namespace test {
class TestProcess : public testing::Test {
protected:
    static void SetUpTestCase() {
    }

    static void TearDownTestCase() {
        //system(cmd.c_str());
    }
};

int CloneRutine(void*) {
    int n = 1;

    while (n--) {
        std::cerr << "running ...\n";
        std::cout << "running ...\n";
        //sleep(1);
    }

    return 10;
}

TEST_F(TestProcess, Clone) {
    baidu::galaxy::container::Process process;
    process.RedirectStderr("./stderr");
    process.RedirectStdout("./stdout");
    pid_t pid = process.Clone(boost::bind(CloneRutine, _1), NULL, 0);
    EXPECT_GT(pid, 0) << pid;
    int status = -1;
    process.Wait(status);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(10, WEXITSTATUS(status));
}


}
}
}
#endif
