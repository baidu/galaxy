// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

namespace baidu {
namespace galaxy {
namespace container {

class ContainerGc {
public:
    ContainerGc();
    ~ContainerGc();

    void Push();

private:
    //std::priority_queue<>
    // gc threadpool
    //

};
}
}
}