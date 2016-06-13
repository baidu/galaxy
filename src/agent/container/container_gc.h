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
    baidu::galaxy::util::ErrorCode Load();
    void Gc(boost::shared_ptr<baidu::galaxy::container::Container> container);
private:
    std::string GcId();

    boost::mutex mutex_;

};
}
}
}
