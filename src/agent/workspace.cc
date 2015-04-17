// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/workspace.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <pwd.h>
#include "common/logging.h"

namespace galaxy {

int DefaultWorkspace::Create() {
    LOG(INFO, "create workspace for task %d", m_task_info.task_id());
    if (m_has_created) {
        return 0;
    }
    //TODO safe path join
    // check users dir
    std::stringstream private_path;
    private_path << "/home/users/";
    int status = 0;
    status = mk_patch(private_path.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (0 != status) {
        LOG(WARNING, "create orient path failed %s err[%d: %s]",
                private_path.str().c_str(), errno, strerror(errno));
    }
    //create acct
    private_path << m_task_info.acct();
    passwd *pw = getpwnam(m_task_info.acct().c_str());
    if (NULL == pw) {
        std::stringstream add_user;
        add_user << "useradd -d " << private_path.str().c_str() << " -m " << m_task_info.acct().c_str();
        system(add_user.str().c_str());
        if (errno) {
            LOG(WARNING, "create acct failed %s err[%d: %s]",
                m_task_info.acct().c_str(), errno, strerror(errno));
        }
    }

    //create work dir
    status = mk_patch(private_path.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    m_task_root_path = private_path.str();
    if (0 != status) {
        LOG(WARNING, "create task root path failed %s err[%d: %s]",
                m_task_root_path.c_str(), errno, strerror(errno));
    }

    private_path << "/data";
    status = mk_patch(private_path.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    m_task_root_path = private_path.str();
    if (0 != status) {
        LOG(WARNING, "create task root path failed %s err[%d: %s]",
                m_task_root_path.c_str(), errno, strerror(errno));
    }

    private_path << "/" << m_task_info.task_id();
    status = mk_patch(private_path.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (status == 0) {
        m_task_root_path = private_path.str();
        m_has_created = true;
    } else {
        LOG(WARNING, "create task root path failed %s err[%d: %s]",
                m_task_root_path.c_str(), errno, strerror(errno));
    }
    return status;
}

int DefaultWorkspace::mk_patch(const char *path, mode_t mode)
{
    struct stat st = {0};
    int status = 0;
    if (stat(path, &st) != 0) {
        if (mkdir(path, mode) != 0 && errno != EEXIST) {
            status = -1;
        }
    }
    else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }
    return status;
}

std::string DefaultWorkspace::GetPath() {
    return m_task_root_path;
}

int DefaultWorkspace::Clean() {
    LOG(INFO,"clean task %d workspace",m_task_info.task_id());
    if (!m_has_created) {
        return 0;
    }
    struct stat st;
    int ret = 0;
    if (stat(m_task_root_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        std::string rm_cmd ;
        rm_cmd = "rm -rf " + m_task_root_path;
        ret = system(rm_cmd.c_str());
        if (ret == 0) {
            LOG(INFO,"clean task %d workspace successfully",m_task_info.task_id());
            m_has_created = false;
        }
    }
    return ret;
}
bool DefaultWorkspace::IsExpire() {
    return false;
}

TaskInfo DefaultWorkspace::GetTaskInfo() {
    return m_task_info;
}

}



