// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _SRC_UTILS_BUILD_INFO_H
#define _SRC_UTILS_BUILD_INFO_H

namespace baidu {
namespace galaxy {

const char* GetBuildTime() {
#if defined(__DATE__) && defined(__TIME__) 
    const char* build_time = __DATE__" "__TIME__;
#else 
    const char* build_time = "unknow build time";
#endif
    return build_time;
}

const char* GetVersion() {
#ifdef VERSION
    const char* version = VERSION;
#else 
    const char* version = "unknow version";
#endif 
    return version;
}

}   // ending namespace galaxy
}   // ending namespace baidu

#endif  //_SRC_UTILS_BUILD_INFO_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
