/* 
 * File:   container.h
 * Author: haolifei
 *
 * Created on 2016年4月3日, 下午8:16
 */

#pragma once
#include <sys/types.h>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include <string>
#include <vector>


namespace baidu {
    namespace galaxy {
        namespace container {
            
            class Container {
            public:
                std::string Id() const;
                int Construct();
                int Destroy();
                int Start();
                int Tasks(std::vector<pid_t>& pids);
                int Pids(std::vector<pid_t>& pids);
                boost::shared_ptr<google::protobuf::Message> Status();
                
            private:
                int Attach(pid_t pid);
                int Detach(pid_t pid);
                int Detachall();
                
            };
            
        }
    }
}
