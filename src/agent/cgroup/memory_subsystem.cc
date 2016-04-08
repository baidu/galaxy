#include "memory_subsystem.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

namespace baidu {
    namespace galaxy {
        namespace cgroup {

            MemorySubsystem::MemorySubsystem(boost::shared_ptr<Option> op) :
                _op(op) {
            }

            MemorySubsystem::~MemorySubsystem() {

            }

            std::string MemorySubsystem::Name() {
                return "memory";
            }

            int MemorySubsystem::Construct() {
                boost::filesystem::path path(_op->Path());

                if (!boost::filesystem::exists(_op->Path()) && boost::filesystem::create_directories(path)) {
                    return -1;
                }

                boost::filesystem::path memory_limit_path(_op->Path());
                memory_limit_path.append("memory.limit_in_bytes");
                 if (0 != baidu::galaxy::cgroup::Attach(memory_limit_path.c_str(), _op->LimitInBytes())) {
                     return -1;
                 }
                
                int64_t excess_mode = _op->Excess() ? 1L : 0L;
                boost::filesystem::path excess_mode_path(_op->Path());
                excess_mode_path.append("memory.excess_mode");
                if (0 != baidu::galaxy::cgroup::Attach(memory_limit_path.c_str(), excess_mode)) {
                     return -1;
                 }
                
                boost::filesystem::path kill_mode_path(_op->Path());
                kill_mode_path.append("memory.kill_mode");
                if (0 != baidu::galaxy::cgroup::Attach(kill_mode_path.c_str(), 0L)) {
                    return -1;
                }
                return 0;
            }

            int MemorySubsystem::Attach(pid_t pid) {
                boost::filesystem::path proc_path;
                proc_path.append("cgroup.procs");
                
                if (!boost::filesystem::exists(proc_path)) {
                    return -1;
                }
                
                return baidu::galaxy::cgroup::Attach(proc_path.c_str(), int64_t(pid));
            }

            boost::shared_ptr<google::protobuf::Message> MemorySubsystem::Status() {
                boost::shared_ptr<google::protobuf::Message> ret;
                return ret;
            }
            
            int MemorySubsystem::GetProcs() {
                boost::filesystem::path proc_path;
                proc_path.append("cgroup.procs");
                
                if (!boost::filesystem::exists(proc_path)) {
                    return -1;
                }
                return 0;
            }
        }
    }
}
