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
    static bool CompareResource(const Resource& r_old, 
                                const Resource& r_new);
    template<class T, class Compare>
    static bool CompareVector(std::vector<T>& l, 
                              std::vector<T>& r,
                              Compare comp) {
        if (l.size() != r.size()) {
            return false;
        }
        std::sort(l.begin(), l.end(), comp);
        std::sort(r.begin(), r.end(), comp);
        for (int i = 0; i < r.size(); i++) {
            // has the same value
            if (!comp(l[i], r[i]) && !comp(r[i], l[i])){
                continue;
            }
            return false;
        }
        // has the same value
        return true;
    }

private:
    static std::string UUID();
};

}   
}

#endif
