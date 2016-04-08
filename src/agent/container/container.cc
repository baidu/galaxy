// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "container.h"

namespace baidu {
namespace galaxy {
namespace container {

std::string Container::Id() const {
    return "";
}

int Container::Construct() {
    return 0;
}

int Container::Destroy() {
    return 0;
}

int Container::Start() {
    return 0;
}

int Container::Tasks(std::vector<pid_t>& pids) {
    return -1;
}

int Container::Pids(std::vector<pid_t>& pids) {

    return -1;
}

boost::shared_ptr<google::protobuf::Message> Container::Status() {
    boost::shared_ptr<google::protobuf::Message> ret;
    return ret;
}

int Container::Attach(pid_t pid) {
    return -1;
}

int Container::Detach(pid_t pid) {
    return -1;
}

int Container::Detachall() {
    return -1;
}

} //namespace container
} //namespace galaxy
} //namespace baidu

