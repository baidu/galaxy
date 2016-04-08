/* 
 * File:   CpuSubsystem.h
 * Author: haolifei
 *
 * Created on 2016年4月3日, 下午4:05
 */

#pragma once
#include "cgroup.h"

#include <stdint.h>

namespace baidu {
    namespace galaxy {
        namespace cgroup {
            
            class CpuSubsystem : public Cgroup {
            public:
                class Option {
                public:
                    Option() :
                        _milli_core(-1),
                        _exceeded(false) {}
                        
                    Option* SetPath(const std::string& path) {
                        _path = path;
                        return this;
                    }
                    
                    Option* SetMilliCore(int64_t millicore) {
                        _milli_core = millicore;
                        return this;
                    }
                    
                    Option* EnableExceeded(bool enable) {
                        _exceeded = enable;
                        return this;
                    }
                    
                    const std::string& Path() const {
                        return _path;
                    }
                    
                    int64_t Millicore() const {
                        return _milli_core;
                    }
                    
                    bool Exceeded() const {
                        return _exceeded;
                    }
                    
                private:
                    std::string _path;
                    int64_t _milli_core;
                    bool _exceeded;
                };
            public:
                CpuSubsystem(boost::shared_ptr<Option> op);
                ~CpuSubsystem();
                
                std::string Name();
                int Construct();
                int Destroy();
                int Attach(pid_t pid);
                boost::shared_ptr<Cgroup> Clone();
                boost::shared_ptr<google::protobuf::Message> Status();
                
            private:
                
                const std::string _name;
                boost::shared_ptr<Option> _op;
            };
        }
    }
}
