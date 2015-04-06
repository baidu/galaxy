// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "mapreduce.h"

#include <iostream>

int main(int argc, char* argv[]) {
    ReduceInput input;
    while(!input.alldone()) {
        std::cout << input.key() << "\t";
        g_reducer->Reduce(&input);
        input.NextKey();
    }
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
