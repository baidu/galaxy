
#include "cgroup.h"

#include <boost/lexical_cast/lexical_cast_old.hpp>

#include <stdio.h>

namespace baidu {
    namespace galaxy {
        namespace cgroup {
            const static int64_t CFS_PERIOD = 100000L;   // cpu.cfs_period_us
            const static int64_t CFS_SHARE = 1000L;      // unit
            const static int64_t MILLI_CORE = 1000L;     // unit

            int Attach(const std::string& file, int64_t value) {
                return Attach(file, boost::lexical_cast<std::string>(value));
            }

            int Attach(const std::string& file, const std::string& value) {
                FILE* fd = ::fopen(file.c_str(), "we");
                if (NULL == fd) {
                    return -1;
                }

                int ret = ::fprintf(fd, "%s", value.c_str());
                ::fclose(fd);
                if (ret <= 0) {
                    return -1;
                }
                return 0;
            }

            int64_t CfsToMilliCore(int64_t cfs) {
                if (cfs <=0) {
                    return -1L;
                }
                return cfs * MILLI_CORE / CFS_PERIOD;
            }

            int64_t ShareToMilliCore(int64_t share) {
                if (share <=0) {
                    return 0;
                }
                return share * MILLI_CORE / CFS_SHARE;
            }

            int64_t MilliCoreToCfs(int64_t millicore) {
                if (millicore <=0) {
                    return -1L;
                }
                return millicore * CFS_PERIOD / MILLI_CORE;
            }

            int64_t MilliCoreToShare(int64_t millicore) {
                if (millicore <=0) {
                    return 0L;
                }
                return millicore * CFS_SHARE / MILLI_CORE;
            }


        }
    }
}
