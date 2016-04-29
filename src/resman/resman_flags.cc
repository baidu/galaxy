#include <gflags/gflags.h>

DEFINE_string(resman_port, "1645", "resman listen port");
DEFINE_int64(sched_interval, 50, "scheduling interval (ms)");
DEFINE_int64(job_gc_check_interval, 60000, "job gc check interval (ms)");
