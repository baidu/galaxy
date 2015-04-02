// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "mapreduce.h"

int main(int argc, char* argv[]) {
    MapInput input;
    while(!input.done()) {
        g_mapper->Map(input);
        input.NextValue();
    }
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
