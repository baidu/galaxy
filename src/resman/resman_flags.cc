#include <gflags/gflags.h>

DEFINE_string(resman_port, "1645", "resman listen port");
DEFINE_int64(sched_interval, 50, "scheduling interval (ms)");
DEFINE_int64(group_gc_check_interval, 60000, "container group gc check interval (ms)");
DEFINE_string(nexus_root, "/galaxy3", "root prefix on nexus");
DEFINE_string(nexus_addr, "", "nexus server list");
