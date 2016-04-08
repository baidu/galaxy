/* 
 * File:   cgroup_factory.h
 * Author: haolifei
 *
 * Created on 2016年4月3日, 下午7:53
 */

#pragma once
#include "cgroup.h"

#include <boost/shared_ptr.hpp>

#include <map>

namespace baidu {
    namespace galaxy {
        namespace cgroup {
            
            class CgroupFactory {
            public:
                void Register(const std::string& name, boost::shared_ptr<Cgroup> subsystem);
                boost::shared_ptr<Cgroup> CreateCgroup(const std::string& name);
                
             private:
                std::map<const std::string, boost::shared_ptr<Cgroup> > _m_cgroup;
            };
        }
    }
}
