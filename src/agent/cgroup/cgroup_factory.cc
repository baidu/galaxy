#include "cgroup_factory.h"

namespace baidu {
    namespace galaxy {
        namespace cgroup {
            
            void CgroupFactory::Register(const std::string& name, boost::shared_ptr<Cgroup> cgroup) {
                assert(NULL != cgroup.get());
                _m_cgroup[name] = cgroup;
            }
            
            boost::shared_ptr<Cgroup> CgroupFactory::CreateCgroup(const std::string& cgroup_config) {
                boost::shared_ptr<Cgroup> ret;
                std::map<std::string, boost::shared_ptr<Cgroup> >::iterator iter = _m_cgroup.find(cgroup_config);
                if (_m_cgroup.end() != iter) {
                    ret = iter->second->Clone();
                }
                
                return ret;
            }
                
        }
    }
}
