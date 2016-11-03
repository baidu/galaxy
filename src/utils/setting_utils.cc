// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "setting_utils.h"

#include <glog/logging.h>

namespace baidu {
namespace galaxy {

void SetupLog(const std::string& name) {
    // log info/warning/error/fatal to galaxy.log
    // log warning/error/fatal to galaxy.wf
    std::string program_name = "galaxy";
    if (!name.empty()) {
        program_name = name;
    }
    if (FLAGS_log_dir.empty()){
        FLAGS_log_dir = ".";
    }
    std::string log_filename = FLAGS_log_dir + "/" + program_name + ".INFO.";
    std::string wf_filename = FLAGS_log_dir + "/" + program_name + ".WARNING.";
    std::string x_filename = FLAGS_log_dir + "/" + program_name + ".888WARNING.";
    google::SetLogDestination(google::INFO, log_filename.c_str());
    google::SetLogDestination(google::WARNING, wf_filename.c_str());
    google::SetLogDestination(google::ERROR, "");
    google::SetLogDestination(google::FATAL, "");
    google::SetLogDestination(10, x_filename.c_str());

    google::SetLogSymlink(google::INFO, program_name.c_str());
    google::SetLogSymlink(google::WARNING, program_name.c_str());
    google::SetLogSymlink(google::ERROR, "");
    google::SetLogSymlink(google::FATAL, "");
    google::InstallFailureSignalHandler();
}

} //namespace galaxy
} //namespace baidu
