// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <cstdlib>
#include <boost/filesystem.hpp>

#include "boost/system/error_code.hpp"
#include "boost/type_traits/integral_constant.hpp"
#include "boost/filesystem/operations.hpp"
#include <iostream>
using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    boost::system::error_code ec;
    if (!boost::filesystem::exists(argv[1], ec) && !boost::filesystem::create_directories(argv[1], ec)) {
        std::cerr << argv[1] << "do exist " << ec.value() << " " << ec.message() << std::endl;
    } else {
        std::cerr << argv[1] << "exist" << ec.value() << " " << ec.message() << std::endl;
    }
    return 0;
}

