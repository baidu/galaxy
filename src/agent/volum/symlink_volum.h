// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "volum.h"

namespace baidu {
namespace galaxy {
namespace volum {

class SymlinkVolum : public Volum {
public:
    SymlinkVolum();
    ~SymlinkVolum();

    int Construct();
    int Destroy();
    int64_t Used();
    std::string ToString();

};

}
}
}
