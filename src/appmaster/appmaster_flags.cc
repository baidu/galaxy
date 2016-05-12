#include <gflags/gflags.h>

// nexus
DEFINE_string(nexus_servers, "127.0.1.1:8868", "server list of nexus, e.g abc.com:1234,def.com:5342");
DEFINE_string(nexus_root_path, "/baidu/galaxy", "root path of galaxy cluster on nexus, e.g /ps/galaxy");

// appmaster
DEFINE_string(appmaster_port, "8220", "appmaster port");
DEFINE_string(appmaster_nexus_path, "appmaster", "appmaster endpoint");
DEFINE_string(appmaster_endpoint, "127.0.1.1:8220", "appmaster endpoint");

