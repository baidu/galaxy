// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include <stdio.h>
#include <stdlib.h>

#include "agent/utils.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "./agent_utils_remove_test path\n"); 
        return -1;
    }

    if (!galaxy::file::Remove(argv[1])) {
        return -1; 
    }

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
