/* 
 * File:   newfile.h
 * Author: haolifei
 *
 * Created on 2016年4月3日, 下午3:46
 */

#pragma once
#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include <sys/types.h>

#include <string>

namespace baidu {
    namespace galaxy {
        namespace cgroup {
            
            class Cgroup {
            public:
                Cgroup() {}
                virtual ~Cgroup() {}
                
                virtual std::string Name() = 0;
                virtual int Construct() = 0;
                virtual int Destroy() = 0;
                virtual int Attach(pid_t pid) = 0;
                virtual boost::shared_ptr<google::protobuf::Message> Status() = 0;
                virtual boost::shared_ptr<Cgroup> Clone() = 0;
                virtual int GetProcs() = 0;
            };
            
            int Attach(const std::string& file, int64_t value);
            int Attach(const std::string& file, const std::string& value);
            
            int64_t CfsToMilliCore(int64_t cfs);
            int64_t ShareToMilliCore(int64_t share);
            int64_t MilliCoreToCfs(int64_t millicore);
            int64_t MilliCoreToShare(int64_t millicore);
        }
    }
}
