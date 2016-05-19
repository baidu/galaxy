// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

namespace baidu {
namespace galaxy {
namespace test {

class UnitTest : public testing::Environment {
    virtual void SetUp() {
    }

    virtual void TeraDown() {
    }
};
}
}
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
