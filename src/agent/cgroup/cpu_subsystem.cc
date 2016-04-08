#include "cpu_subsystem.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>

#include <assert.h>

namespace baidu {
    namespace galaxy {
        namespace cgroup {
            CpuSubsystem::CpuSubsystem(boost::shared_ptr<Option> op) :
                _name("cpu"),
                _op(op) {
                assert(op.get() != NULL);
            }
            
            CpuSubsystem::~CpuSubsystem() {
                
            }

            std::string CpuSubsystem::Name() {
                return _name;
            }
            
            int CpuSubsystem::Construct() {
                boost::filesystem::path path(_op->Path());
                int ret = -1;
                if (!boost::filesystem::exists(_op->Path()) && boost::filesystem::create_directories(path)) {
                    return ret;
                }
                
                if (_op->Exceeded()) {
                    boost::filesystem::path cpu_share(_op->Path());
                    cpu_share.append("cpu.shares");
                    
                    boost::filesystem::path cpu_cfs(_op->Path());
                    cpu_cfs.append("cpu.cfs_quota_us");
                    
                    if (0 == baidu::galaxy::cgroup::Attach(cpu_share.c_str(), MilliCoreToShare(_op->Millicore())) 
                            && 0 == baidu::galaxy::cgroup::Attach(cpu_cfs.c_str(), -1)) {
                        ret = 0;
                    }
                    
                } else {
                    boost::filesystem::path cpu_cfs(_op->Path());
                    if (0 == baidu::galaxy::cgroup::Attach(cpu_cfs.c_str(), MilliCoreToCfs(_op->Millicore()))) {
                        ret = 0;
                    }                    
                }
                return ret;
            }
            
            int CpuSubsystem::Destroy() {
                return 0;
            }
            
            int CpuSubsystem::Attach(pid_t pid) {
                boost::filesystem::path proc_path;
                proc_path.append("cgroup.procs");
                
                if (!boost::filesystem::exists(proc_path)) {
                    return -1;
                }
                
                return baidu::galaxy::cgroup::Attach(proc_path.c_str(), int64_t(pid));
            }
            
            boost::shared_ptr<google::protobuf::Message> CpuSubsystem::Status() {
                boost::shared_ptr<google::protobuf::Message> ret;
                return ret;
            }


            boost::shared_ptr<Cgroup> CpuSubsystem::Clone() {
                //boost::shared_ptr<Cgroup> ret(new CpuSubsystem());
                boost::shared_ptr<Cgroup> ret;
                return ret;
            }
            
        }
    }
}
