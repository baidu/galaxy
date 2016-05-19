#include <gflags/gflags.h>

DEFINE_string(agent_port, "1646", "agent listen port");
DEFINE_string(mount_templat, "/proc,/etc,/home,/usr,/var,/dev,/boot,/bin", "mount templat");
DEFINE_string(cgroup_root_path, "/cgroup", "cgroup root path");
