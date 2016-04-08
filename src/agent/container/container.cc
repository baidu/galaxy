#include "container.h"

namespace baidu {
    namespace galaxy {
        namespace container {
            std::string Container::Id() const {
                return "";
            }
            
            int Container::Construct() {
                return 0;
            }
            
            int Container::Destroy() {
                return 0;
            }
            
            int Container::Start() {
                return 0;
            }
            
            int Container::Tasks(std::vector<pid_t>& pids) {
                return -1;
            }

            int Container::Pids(std::vector<pid_t>& pids) {

                return -1;
            }

            boost::shared_ptr<google::protobuf::Message> Container::Status() {
                boost::shared_ptr<google::protobuf::Message> ret;
                return ret;
            }

            int Container::Attach(pid_t pid) {
                return -1;
            }

            int Container::Detach(pid_t pid) {
                return -1;
            }

            int Container::Detachall() {
                return -1;
            }
        }
    }
}
