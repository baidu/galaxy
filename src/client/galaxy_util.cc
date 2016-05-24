// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <cstring>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include "sdk/galaxy_sdk.h"

namespace baidu {
namespace galaxy {
namespace client {

//初始化字符串映射表
std::string StringAuthority(const ::baidu::galaxy::sdk::Authority& authority) {
    std::string result;
    switch(authority) {
    case ::baidu::galaxy::sdk::kAuthorityCreateContainer:
        result = "kAuthorityCreateContainer";
        break;
    case ::baidu::galaxy::sdk::kAuthorityRemoveContainer:
        result = "kAuthorityRemoveContainer";
        break;
    case ::baidu::galaxy::sdk::kAuthorityUpdateContainer:
        result = "kAuthorityUpdateContainer";
        break;
    case ::baidu::galaxy::sdk::kAuthorityListContainer:
        result = "kAuthorityListContainer";
        break;
    case ::baidu::galaxy::sdk::kAuthoritySubmitJob:
        result = "kAuthoritySubmitJob";
        break;
    case ::baidu::galaxy::sdk::kAuthorityRemoveJob:
        result = "kAuthorityRemoveJob";
        break;
    case ::baidu::galaxy::sdk::kAuthorityUpdateJob:
        result = "kAuthorityUpdateJob";
        break;
    case ::baidu::galaxy::sdk::kAuthorityListJobs:
        result = "kAuthorityListJobs";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringAuthorityAction(const ::baidu::galaxy::sdk::AuthorityAction& action) {
    std::string result;
    switch(action) {
    case ::baidu::galaxy::sdk::kActionAdd:
        result = "kActionAdd";
        break;
    case ::baidu::galaxy::sdk::kActionRemove:
        result = "kActionRemove";
        break;
    case ::baidu::galaxy::sdk::kActionSet:
        result = "kActionSet";
        break;
    case ::baidu::galaxy::sdk::kActionClear:
        result = "kActionClear";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringVolumMedium(const ::baidu::galaxy::sdk::VolumMedium& medium) {
    std::string result;
    switch(medium) {
    case ::baidu::galaxy::sdk::kSsd:
        result = "kSsd";
        break;
    case ::baidu::galaxy::sdk::kDisk:
        result = "kDisk";
        break;
    case ::baidu::galaxy::sdk::kBfs:
        result = "kBfs";
        break;
    case ::baidu::galaxy::sdk::kTmpfs:
        result = "kTmpfs";
        break;
    defalt:
        result = "";
    }
    return result;

}
std::string StringVolumType(const ::baidu::galaxy::sdk::VolumType& type) {
    std::string result;
    switch(type) {
    case ::baidu::galaxy::sdk::kEmptyDir:
        result = "kEmptyDir";
        break;
    case ::baidu::galaxy::sdk::kHostDir:
        result = "kHostDir";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringJobType(const ::baidu::galaxy::sdk::JobType& type) {

    std::string result;
    switch(type) {
    case ::baidu::galaxy::sdk::kJobMonitor:
        result = "kJobMonitor";
        break;
    case ::baidu::galaxy::sdk::kJobService:
        result = "kJobService";
        break;
    case ::baidu::galaxy::sdk::kJobBatch:
        result = "kJobBatch";
        break;
    case ::baidu::galaxy::sdk::kJobBestEffort:
        result = "kJobBestEffort";
        break;
    defalt:
        result = "";
    }
    return result;

}

std::string StringJobStatus(const ::baidu::galaxy::sdk::JobStatus& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kJobPending:
        result = "kJobPending";
        break;
    case ::baidu::galaxy::sdk::kJobRunning:
        result = "kJobRunning";
        break;
    case ::baidu::galaxy::sdk::kJobFinished:
        result = "kJobFinished";
        break;
    case ::baidu::galaxy::sdk::kJobDestroying:
        result = "kJobDestroying";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringPodStatus(const ::baidu::galaxy::sdk::PodStatus& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kPodPending:
        result = "kPodPending";
        break;
    case ::baidu::galaxy::sdk::kPodReady:
        result = "kPodReady";
        break;
    case ::baidu::galaxy::sdk::kPodDeploying:
        result = "kPodDeploying";
        break;
    case ::baidu::galaxy::sdk::kPodStarting:
        result = "kPodStarting";
        break;
    case ::baidu::galaxy::sdk::kPodServing:
        result = "kPodServing";
        break;
    case ::baidu::galaxy::sdk::kPodFailed:
        result = "kPodFailed";
        break;
    case ::baidu::galaxy::sdk::kPodFinished:
        result = "kPodFinished";
        break;
    defalt:
        result = "";
    }
    return result;

}

std::string StringTaskStatus(const ::baidu::galaxy::sdk::TaskStatus& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kTaskPending:
        result = "kTaskPending";
        break;
    case ::baidu::galaxy::sdk::kTaskDeploying:
        result = "kTaskDeploying";
        break;
    case ::baidu::galaxy::sdk::kTaskStarting:
        result = "kTaskStarting";
        break;
    case ::baidu::galaxy::sdk::kTaskServing:
        result = "kTaskServing";
        break;
    case ::baidu::galaxy::sdk::kTaskFailed:
        result = "kTaskFailed";
        break;
    case ::baidu::galaxy::sdk::kTaskFinished:
        result = "kTaskFinished";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringContainerStatus(const ::baidu::galaxy::sdk::ContainerStatus& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kContainerPending:
        result = "kContainerPending";
        break;
    case ::baidu::galaxy::sdk::kContainerAllocating:
        result = "kContainerAllocating";
        break;
    case ::baidu::galaxy::sdk::kContainerReady:
        result = "kContainerReady";
        break;
    case ::baidu::galaxy::sdk::kContainerFinish:
        result = "kContainerFinish";
        break;
    case ::baidu::galaxy::sdk::kContainerError:
        result = "kContainerError";
        break;
    case ::baidu::galaxy::sdk::kContainerDestroying:
        result = "kContainerDestroying";
        break;
    case ::baidu::galaxy::sdk::kContainerTerminated:
        result = "kContainerTerminated";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringContainerGroupStatus(const ::baidu::galaxy::sdk::ContainerGroupStatus& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kContainerGroupNormal:
        result = "kContainerGroupNormal";
        break;
    case ::baidu::galaxy::sdk::kContainerGroupTerminated:
        result = "kContainerGroupTerminated";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringStatus(const ::baidu::galaxy::sdk::Status& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kOk:
        result = "kOk";
        break;
    case ::baidu::galaxy::sdk::kError:
        result = "kError";
        break;
    case ::baidu::galaxy::sdk::kTerminate:
        result = "kTerminate";
        break;
    case ::baidu::galaxy::sdk::kAddAgentFail:
        result = "kAddAgentFail";
        break;
    case ::baidu::galaxy::sdk::kDeny:
        result = "kDeny";
        break;
    case ::baidu::galaxy::sdk::kJobNotFound:
        result = "kJobNotFound";
        break;
    case ::baidu::galaxy::sdk::kCreateContainerGroupFail:
        result = "kCreateContainerGroupFail";
        break;
    case ::baidu::galaxy::sdk::kRemoveContainerGroupFail:
        result = "kRemoveContainerGroupFail";
        break;
    case ::baidu::galaxy::sdk::kUpdateContainerGroupFail:
        result = "kUpdateContainerGroupFail";
        break;
    case ::baidu::galaxy::sdk::kRemoveAgentFail:
        result = "kRemoveAgentFail";
        break;
    case ::baidu::galaxy::sdk::kCreateTagFail:
        result = "kCreateTagFail";
        break;
    case ::baidu::galaxy::sdk::kAddAgentToPoolFail:
        result = "kAddAgentToPoolFail";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringAgentStatus(const ::baidu::galaxy::sdk::AgentStatus& status) {
    std::string result;
    switch(status) {
    case ::baidu::galaxy::sdk::kAgentUnkown:
        result = "kAgentUnkown";
        break;
    case ::baidu::galaxy::sdk::kAgentAlive:
        result = "kAgentAlive";
        break;
    case ::baidu::galaxy::sdk::kAgentDead:
        result = "kAgentDead";
        break;
    case ::baidu::galaxy::sdk::kAgentOffline:
        result = "kAgentOffline";
        break;
    defalt:
        result = "";
    }
    return result;
}

std::string StringResourceError(const ::baidu::galaxy::sdk::ResourceError& error) {
    std::string result;
    switch(error) {
    case ::baidu::galaxy::sdk::kResOk:
        result = "kResOk";
        break;
    case ::baidu::galaxy::sdk::kNoCpu:
        result = "kNoCpu";
        break;
    case ::baidu::galaxy::sdk::kNoMemory:
        result = "kNoMemory";
        break;
    case ::baidu::galaxy::sdk::kNoMedium:
        result = "kNoMedium";
        break;
    case ::baidu::galaxy::sdk::kNoDevice:
        result = "kNoDevice";
        break;
    case ::baidu::galaxy::sdk::kNoPort:
        result = "kNoPort";
        break;
    case ::baidu::galaxy::sdk::kPortConflict:
        result = "kPortConflict";
        break;
    case ::baidu::galaxy::sdk::kTagMismatch:
        result = "kTagMismatch";
        break;
    case ::baidu::galaxy::sdk::kNoMemoryForTmpfs:
        result = "kNoMemoryForTmpfs";
        break;
    case ::baidu::galaxy::sdk::kPoolMismatch:
        result = "kPoolMismatch";
        break;
    case ::baidu::galaxy::sdk::kTooManyPods:
        result = "kTooManyPods";
        break;
    defalt:
        result = "";
    }
    return result;

}

std::string FormatDate(uint64_t datetime) {
    if (datetime < 100) {
        return "-";
    }

    char buf[100];
    time_t time = datetime / 1000000;
    struct tm *tmp;
    tmp = localtime(&time);
    strftime(buf, 100, "%F %X", tmp);
    std::string ret(buf);
    return ret;
}

bool GetHostname(std::string* hostname) {
    char buf[100];
    if (gethostname(buf, 100) != 0) {
        fprintf(stderr, "gethostname failed\n");
        return false;
    }
    *hostname = buf;
    return true;
}

//单位转换
int UnitStringToByte(const std::string& input, int64_t* output) {
    if (output == NULL) {
        return -1;
    }

    std::map<char, int32_t> subfix_table;
    subfix_table['K'] = 1;
    subfix_table['M'] = 2;
    subfix_table['G'] = 3;
    subfix_table['T'] = 4;
    subfix_table['B'] = 5;
    subfix_table['Z'] = 6;

    int64_t num = 0;
    char subfix = 0;
    int32_t shift = 0;
    int32_t matched = sscanf(input.c_str(), "%ld%c", &num, &subfix);
    if (matched <= 0) {
        fprintf(stderr, "unit sscanf failed\n");
        return -1;
    }

    if (matched == 2) {
        std::map<char, int32_t>::iterator it = subfix_table.find(subfix);
        if (it == subfix_table.end()) {
            fprintf(stderr, "unit is error, it must be in [K, M, G, T, B, Z]\n");
            return -1;
        }
        shift = it->second;
    }

    while (shift > 0) {
        num *= 1024;
        shift--;
    }
    *output = num;
    return 0;
}

//读取endpoint
bool LoadAgentEndpointsFromFile(const std::string& file_name, std::vector<std::string>* agents) {
    const int LINE_BUF_SIZE = 1024;
    char line_buf[LINE_BUF_SIZE];
    std::ifstream fin(file_name.c_str(), std::ifstream::in);        
    if (!fin.is_open()) {
        fprintf(stderr, "open %s failed\n", file_name.c_str());
        return false; 
    }
    
    bool ret = true;
    int i = 0;
    while (!fin.eof() && fin.good()) {
        fin.getline(line_buf, LINE_BUF_SIZE);     
        if (fin.gcount() == LINE_BUF_SIZE) {
            fprintf(stderr, "line buffer size overflow\n");     
            ret = false;
            break;
        } else if (fin.gcount() == 0 || strlen(line_buf) == 0) {
            continue; 
        }
        fprintf(stdout, "endpoint %ld: %s\n", fin.gcount(), line_buf);
        // NOTE string size should == strlen
        std::string agent_endpoint(line_buf, strlen(line_buf));
        agents->push_back(agent_endpoint);
        ++i;
    }

    if (!ret) {
        fin.close();
        return false;
    }

    fin.close(); 
    return true;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu
/* vim: set ts=4 sw=4 sts=4 tw=100 */
