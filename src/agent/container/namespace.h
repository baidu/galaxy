/* 
 * File:   namespace.h
 * Author: haolifei
 *
 * Created on 2016年4月5日, 下午3:31
 */

#pragma once
#ifndef set_ns
#define set_ns(fd) syscall(308, 0, fd)
#endif

namespace baidu {
    namespace galaxy {
        namespace ns {
            int Attach(pid_t pid);
        }
    }
}