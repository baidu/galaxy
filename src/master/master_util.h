// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BAIDU_GALAXY_MASTER_UTIL_H
#define BAIDU_GALAXY_MASTER_UTIL_H

#include <vector>
#include <algorithm> 
#include <string>

namespace baidu {
namespace galaxy {

class JobDescriptor;
class Resource;

class MasterUtil {
public:
    static std::string GenerateJobId(const JobDescriptor& job_desc);
    static std::string GeneratePodId(const JobDescriptor& job_desc);

    static void AddResource(const Resource& from, Resource* to);
    static void SubstractResource(const Resource& from, Resource* to);
    static bool FitResource(const Resource& from, const Resource& to);
    static std::string SelfEndpoint();
    static void TraceJobDesc(const JobDescriptor& job_desc);
private:
    static std::string UUID();
};

}   
}

#endif
