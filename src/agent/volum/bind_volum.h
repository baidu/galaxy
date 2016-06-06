// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "volum.h"
#include "volum_collector.h"

namespace baidu {
namespace galaxy {
namespace volum {

class BindVolum : public Volum {
public:
    BindVolum();
    ~BindVolum();

    baidu::galaxy::util::ErrorCode Construct();
    baidu::galaxy::util::ErrorCode Destroy();
    //baidu::galaxy::util::ErrorCode Gc();
    int64_t Used();
    std::string ToString();

private:
    baidu::galaxy::util::ErrorCode Construct_();
    boost::shared_ptr<VolumCollector> vc_;

};

}
}
}
