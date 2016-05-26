#include <gflags/gflags.h>

DEFINE_string(appmaster_port, "8220", "appmaster port");
DEFINE_string(nexus_addr, "127.0.1.1:8868", "server list of nexus, e.g abc.com:1234,def.com:5342");
DEFINE_string(nexus_root_path, "/baidu/galaxy", "root path of galaxy cluster on nexus, e.g /ps/galaxy");
DEFINE_string(appmaster_nexus_path, "appmaster", "appmaster endpoint");
DEFINE_string(appmaster_endpoint, "cp01-ocean-893.epc.baidu.com:8220", "appmaster endpoint");
