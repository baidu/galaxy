#include <gtest/gtest.h>
#include "src/appworker/utils.h"

TEST(TestUtils, Md5File) {
    EXPECT_EQ(2, 1 + 1);
    std::string md5 = baidu::galaxy::md5::Md5File("/root/workspace/galaxy3/appworker");
    EXPECT_EQ("063b16c2e185fa84cad78a8e0e058673", md5);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    // Runs all tests using Google Test.
    return RUN_ALL_TESTS();
}
