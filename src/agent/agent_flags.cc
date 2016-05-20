#include <gflags/gflags.h>

DEFINE_string(mount_templat, "/proc,/etc,/home,/usr,/var,/dev,/boot,/bin", "mount templat");
DEFINE_string(cgroup_root_path, "/cgroup", "cgroup root path");

DEFINE_string(nexus_root_path, "", "root path on nexus");
DEFINE_string(master_path, "", "master path");
DEFINE_string(nexus_servers, "", "servers of nexus cluster");

DEFINE_string(agent_ip, "", "agent ip");
DEFINE_string(agent_port, "1646", "agent listen port");
DEFINE_int32(keepalive_interval, 5000, "keep alive with RM");
