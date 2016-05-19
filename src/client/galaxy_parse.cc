// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include "galaxy_util.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

namespace baidu {
namespace galaxy {
namespace client {

//单位转换
int UnitStringToByte(const std::string& input, int64_t* output) {
    if (output == NULL) {
        return -1;
    }

    std::map<char, int32_t> subfix_table;
    subfix_table['K'] = 1;
    subfix_table['M'] = 2;
    subfix_table['G'] = 3;
    subfix_table['T'] = 4;
    subfix_table['B'] = 5;
    subfix_table['Z'] = 6;

    int64_t num = 0;
    char subfix = 0;
    int32_t shift = 0;
    int32_t matched = sscanf(input.c_str(), "%ld%c", &num, &subfix);
    if (matched <= 0) {
        fprintf(stderr, "unit sscanf failed\n");
        return -1;
    }

    if (matched == 2) {
        std::map<char, int32_t>::iterator it = subfix_table.find(subfix);
        if (it == subfix_table.end()) {
            fprintf(stderr, "unit is error, it must be in [K, M, G, T, B, Z]\n");
            return -1;
        }
        shift = it->second;
    }

    while (shift > 0) {
        num *= 1024;
        shift--;
    }
    *output = num;
    return 0;
}

int ParseDeploy(const rapidjson::Value& deploy_json, ::baidu::galaxy::sdk::Deploy* deploy) {
    //deploy config:replica
    if (!deploy_json.HasMember("replica") || deploy_json["replica"].GetInt() < 0) {
        fprintf(stderr, "replica is required in deploy\n");
        return -1;
    }
    deploy->replica = deploy_json["replica"].GetInt();

    //deploy config:step
    if (!deploy_json.HasMember("step") || deploy_json["step"].GetInt() < 0) {
        fprintf(stderr, "step is required in deploy\n");
        return -1;
    }
    deploy->step = deploy_json["step"].GetInt();

    //deploy config:interval
    if (!deploy_json.HasMember("interval") || deploy_json["interval"].GetInt() < 0) {
        fprintf(stderr, "interval is required in deploy\n");
        return -1;
    }
    deploy->interval = deploy_json["interval"].GetInt();

    //deploy config:max_per_host
    if (!deploy_json.HasMember("max_per_host") || deploy_json["max_per_host"].GetInt() <= 0) {
        fprintf(stderr, "max_per_host is required in deploy\n");
        return -1;
    }
    deploy->max_per_host = deploy_json["max_per_host"].GetInt();
    return 0;

}

int ParseVolum(const rapidjson::Value& volum_json, ::baidu::galaxy::sdk::VolumRequired* volum) {
    int ok = 0;
    //size
    if (!volum_json.HasMember("size")) {
        fprintf(stderr, "size is required in volum\n");
        return -1;
    }
    
    ok = UnitStringToByte(volum_json["size"].GetString(), &volum->size);
    if (ok != 0) {
        return -1;
    }

    //type
    if (!volum_json.HasMember("type")) {
        volum->type = ::baidu::galaxy::sdk::kEmptyDir;
    } else {
        std::string type = volum_json["type"].GetString();
        if (type.compare("kEmptyDir") == 0) {
            volum->type = ::baidu::galaxy::sdk::kEmptyDir;
        } else if (type.compare("kHostDir") == 0) {
            volum->type = ::baidu::galaxy::sdk::kHostDir;
        } else {
            fprintf(stderr, "type must be [kEmptyDir, kHostDir] in volum\n");
            return -1;
        }
    }

    //medium
    if (!volum_json.HasMember("medium")) {
        volum->medium = ::baidu::galaxy::sdk::kDisk;
    } else {
        std::string medium = volum_json["medium"].GetString();
        if (medium.compare("kSsd") == 0) {
            volum->medium = ::baidu::galaxy::sdk::kSsd;
        } else if (medium.compare("kDisk") == 0) {
            volum->medium = ::baidu::galaxy::sdk::kDisk;
        } else if (medium.compare("kBfs") == 0) {
            volum->medium = ::baidu::galaxy::sdk::kBfs;
        } else if (medium.compare("kTmpfs") == 0) {
            volum->medium = ::baidu::galaxy::sdk::kTmpfs;
        } else {
        } 
    }

    //source_path
    volum->source_path = volum_json["source_path"].GetString();
    
    //dest_path
    if (!volum_json.HasMember("dest_path")) {
        fprintf(stderr, "dest_path is required in volum\n");
        return -1;
    }
    volum->dest_path = volum_json["dest_path"].GetString();

    //readonly
    if (!volum_json.HasMember("readonly")) {
        volum->readonly = false;
    } else {
        volum->readonly = volum_json["readonly"].GetBool();
    }

    //exclusive
    if (!volum_json.HasMember("exclusive")) {
        volum->exclusive = false;
    } else {
        volum->exclusive = volum_json["exclusive"].GetBool();
    }

    //use_symlink
    if (!volum_json.HasMember("use_symlink")) {
        volum->use_symlink = true;
    } else {
        volum->use_symlink = volum_json["use_symlink"].GetBool();
    }

    return 0;

}

int ParseCpu(const rapidjson::Value& cpu_json, ::baidu::galaxy::sdk::CpuRequired* cpu) {
    if (!cpu_json.HasMember("millicores")) {
        fprintf(stderr, "milli_cores is required in cpu\n");
        return -1;
    }
    cpu->milli_core = cpu_json["millicores"].GetInt();

    if (!cpu_json.HasMember("excess")) {
        cpu->excess = false;
    } else {
        cpu->excess = cpu_json["excess"].GetBool();
    }

    return 0;

}

int ParseMem(const rapidjson::Value& mem_json, ::baidu::galaxy::sdk::MemoryRequired* mem) {
    int ok = 0;
    if (!mem_json.HasMember("size")) {
        fprintf(stderr, "size is required in mem\n");
        return -1;
    }
    
    ok = UnitStringToByte(mem_json["size"].GetString(), &mem->size);
    if (ok != 0) {
        return -1;
    }

    if (!mem_json.HasMember("excess")) {
        mem->excess = false;
    } else {
        mem->excess = mem_json["excess"].GetBool();
    }

    return 0;

}

int ParsePackage(const rapidjson::Value& pack_json, ::baidu::galaxy::sdk::Package* pack) {
    if (!pack_json.HasMember("source_path")) {
        fprintf(stderr, "source_path is required in package\n");
        return -1;
    }
    pack->source_path = pack_json["source_path"].GetString();

    if (!pack_json.HasMember("dest_path")) {
        fprintf(stderr, "dest_path is required in package\n");
        return -1;
    }
    pack->dest_path = pack_json["dest_path"].GetString();

    if (!pack_json.HasMember("version")) {
        fprintf(stderr, "version is required in package\n");
        return -1;
    }
    pack->version = pack_json["version"].GetString();

    return 0;

}

int ParseImagePackage(const rapidjson::Value& image_json, ::baidu::galaxy::sdk::ImagePackage* image) {
    if (!image_json.HasMember("start_cmd")) {
        fprintf(stderr, "start_cmd is required in exec_package\n");
        return -1;
    }
    image->start_cmd = image_json["start_cmd"].GetString();
    
    if (!image_json.HasMember("stop_cmd")) {
        fprintf(stderr, "stop_cmd is required in exec_package\n");
        return -1;
    }
    image->stop_cmd = image_json["stop_cmd"].GetString();
    
    if (!image_json.HasMember("package")) {
        fprintf(stderr, "package is required in exec_package\n");
        return -1;
    }
    const rapidjson::Value& pack_json = image_json["package"];
    ::baidu::galaxy::sdk::Package& pack = image->package;
    return ParsePackage(pack_json, &pack);
}

int ParseDataPackage(const rapidjson::Value& data_json, ::baidu::galaxy::sdk::DataPackage* data) {
    int ok = 0;
    if (!data_json.HasMember("reload_cmd")) {
        fprintf(stderr, "reload_cmd is required in data_package\n");
        return -1;
    }
    data->reload_cmd = data_json["reload_cmd"].GetString();
    if (!data_json.HasMember("packages")) {
        fprintf(stderr, "packages is required in data_package\n");
        return -1;
    }
    
    const rapidjson::Value& packages_json = data_json["packages"];
    if (packages_json.Size() <= 0) {
        fprintf(stderr, "size of packages is zero in data_package\n");
        return -1;
    }

    std::vector< ::baidu::galaxy::sdk::Package>& packs = data->packages;
    for (rapidjson::SizeType i = 0; i < packages_json.Size(); ++i) {
        ::baidu::galaxy::sdk::Package pack;
        ok = ParsePackage(packages_json[i], &pack);
        if (ok != 0) {
            break;
        }
        packs.push_back(pack);
    }

    if (ok != 0) {
        fprintf(stderr, "packages is error in data_package\n");
        return -1;
    }

    return 0;
}

int ParsePort(const rapidjson::Value& port_json, ::baidu::galaxy::sdk::PortRequired* port) {
    if (!port_json.HasMember("name")) {
        fprintf(stderr, "port_name is required in port\n");
        return -1;
    }
    port->port_name = port_json["name"].GetString();
    
    if (!port_json.HasMember("port")) {
        fprintf(stderr, "port is required in port\n");
        return -1;
    }
    port->port = port_json["port"].GetString();
    return 0;

}

int ParseService(const rapidjson::Value& service_json, ::baidu::galaxy::sdk::Service* service) {
    if (!service_json.HasMember("service_name")) {
        fprintf(stderr, "service_name is needed in service\n");
        return -1;
    }
    service->service_name = service_json["service_name"].GetString();

    if (!service_json.HasMember("port_name")) {
        fprintf(stderr, "port_name is needed in service\n");
        return -1;
    }
    service->port_name = service_json["port_name"].GetString();

    if (!service_json.HasMember("use_bns")) {
        service->use_bns = false;
    } else {
        service->use_bns = service_json["use_bns"].GetBool();
    }

    return 0;

}

int ParseTask(const rapidjson::Value& task_json, ::baidu::galaxy::sdk::TaskDescription* task) {
    int ok = 0;

    time_t timestamp;
    time(&timestamp);
    task->id = baidu::common::NumToString(timestamp);

    if (!task_json.HasMember("cpu")) {
        fprintf(stderr, "cpu is required in task\n");
        return -1;
    }
    ::baidu::galaxy::sdk::CpuRequired& cpu = task->cpu;
    ok = ParseCpu(task_json["cpu"], &cpu);
    if (ok != 0) {
        fprintf(stderr, "cpu is errror in task cpu\n");
        return -1;
    }

    if (!task_json.HasMember("mem")) {
        fprintf(stderr, "mem is required in task mem\n");
        return -1;
    }

    ::baidu::galaxy::sdk::MemoryRequired& mem = task->memory;
    ok = ParseMem(task_json["mem"], &mem);
    if (ok != 0) {
        fprintf(stderr, "mem is errror in task mem\n");
        return -1;
    }
    
    std::vector< ::baidu::galaxy::sdk::PortRequired>& ports = task->ports;
    if (task_json.HasMember("ports")) {
        const rapidjson::Value& ports_json = task_json["ports"];
        for (rapidjson::SizeType i = 0; i < ports_json.Size(); ++i) {
            ::baidu::galaxy::sdk::PortRequired port;
            ok = ParsePort(ports_json[i], &port);
            if (ok != 0) {
                break;
            }
            ports.push_back(port);
        }

        if (ok != 0) {
            //fprintf(stderr, "ports[%u] is error in task\n", i);
            return -1;
        }
    }

    if (!task_json.HasMember("exec_package")) {
        fprintf(stderr, "exec_package is needed in task\n");
        return -1;
    }

    ::baidu::galaxy::sdk::ImagePackage& exec_package = task->exe_package;
    ok = ParseImagePackage(task_json["exec_package"], &exec_package);
    if (ok != 0) {
        fprintf(stderr, "data_package is needed in task\n");
        return -1;
    }

    if (task_json.HasMember("data_package")) {
        ::baidu::galaxy::sdk::DataPackage& data_package = task->data_package;
        ok = ParseDataPackage(task_json["data_package"], &data_package);
        if (ok != 0) {
            fprintf(stderr, "data_package is needed in task\n");
            return -1;
        }
    }

    std::vector< ::baidu::galaxy::sdk::Service>& services = task->services;
    if (!task_json.HasMember("services")) {
        fprintf(stderr, "services is needed in task\n");
        return -1;
    }

    const rapidjson::Value& servers_json = task_json["services"];
    if (servers_json.Size() <= 0) {
        fprintf(stderr, "size of services is zero\n");
        return -1;
    }

    for(rapidjson::SizeType i = 0; i < servers_json.Size(); ++i) {
        ::baidu::galaxy::sdk::Service service;
        ok = ParseService(servers_json[i], &service);
        if (ok != 0) {
            break;
        }
        services.push_back(service);
    }
    
    if (ok != 0) {
        //fprintf(stderr, "services[%d] is error in task\n", i);
        return -1;
    } 

    return 0;
}

int ParsePod(const rapidjson::Value& pod_json, ::baidu::galaxy::sdk::PodDescription* pod) {
    int ok = 0;
    if (!pod_json.HasMember("workspace_volum")) {
        fprintf(stderr, "workspace_volum is required in pod\n");
        return -1;
    }

    ::baidu::galaxy::sdk::VolumRequired& workspace_volum =pod->workspace_volum;
    const rapidjson::Value& pod_workspace_volum_json = pod_json["workspace_volum"];

    ok = ParseVolum(pod_workspace_volum_json, &workspace_volum);
    if (ok != 0) {
        fprintf(stderr, "deploy is required in config\n");
        return -1;
    }

    std::vector< ::baidu::galaxy::sdk::VolumRequired>& data_volums = pod->data_volums;
    if (pod_json.HasMember("data_volums")) {
        const rapidjson::Value& data_volums_json = pod_json["data_volums"];
        for(rapidjson::SizeType i = 0; i < data_volums_json.Size(); ++i) {
            ::baidu::galaxy::sdk::VolumRequired data_volum;
            ok = ParseVolum(data_volums_json[i], &data_volum);
            if (ok != 0) {
                break;
            }
            data_volums.push_back(data_volum);
        }
    }

    if (ok != 0) {
        //fprintf(stderr, "data_valums[%d] is error in pod\n", i + 1);
        return -1;
    }

    std::vector< ::baidu::galaxy::sdk::TaskDescription>& tasks = pod->tasks;
    if (!pod_json.HasMember("tasks")) {
        fprintf(stderr, "tasks is required in pod\n");
        return -1;
    }

    const rapidjson::Value& tasks_json = pod_json["tasks"];
    if (tasks_json.Size() == 0) {
        fprintf(stderr, "size of tasks must be larger than 0 in pod\n");
        return -1;
    }

    for (rapidjson::SizeType i = 0; i < tasks_json.Size(); ++i) {
        ::baidu::galaxy::sdk::TaskDescription task;
        ok = ParseTask(tasks_json[i], &task);
        if (ok != 0) {
            break;
        }
        tasks.push_back(task);
    }

    if (ok != 0) {
        //fprintf(stderr, "task[%d] is error in pod\n", i + 1);
        return -1;
    }

    return 0;

}

int ParseDocument(const rapidjson::Document& doc, ::baidu::galaxy::sdk::JobDescription* job) {
    int ok = 0;
    //name
    if (!doc.HasMember("name")) {
        fprintf(stderr, "name is required in config\n");
        return -1;
    }
    job->name = doc["name"].GetString();

    //type
    if (!doc.HasMember("type")) {
        fprintf(stderr, "type is required in config\n");
        return -1;
    }
    
    std::string type = doc["type"].GetString();
    if (type.compare("kJobMonitor") == 0) {
        job->type = ::baidu::galaxy::sdk::kJobMonitor;
    }else if (type.compare("kJobService") == 0) {
        job->type = ::baidu::galaxy::sdk::kJobService;
    } else if (type.compare("kJobBatch") == 0) {
        job->type = ::baidu::galaxy::sdk::kJobBatch;
    } else if (type.compare("kJobBestEffort") == 0) {
        job->type = ::baidu::galaxy::sdk::kJobBestEffort;
    } else {
        fprintf(stderr, "type is error, it must be [kJobMonitor,kJobService,kJobBatch,kJobBestEffort]\n");
        return -1;
    }

    //version
    if (!doc.HasMember("version")) {
        fprintf(stderr, "version is required in config\n");
        return -1;
    }
    job->version = doc["version"].GetString();

    ::baidu::galaxy::sdk::Deploy& deploy = job->deploy;

    //deploy
    if (!doc.HasMember("deploy")) {
        fprintf(stderr, "deploy is required in config\n");
        return -1;
    }
    const rapidjson::Value& deploy_json = doc["deploy"];

    ok = ParseDeploy(deploy_json, &deploy);
    if (ok != 0) {
        fprintf(stderr, "deploy is required in config\n");
        return -1;
    }
    
    //pod
    if (!doc.HasMember("pod")) {
        fprintf(stderr, "pod is required in config\n");
        return -1;
    }
    const rapidjson::Value& pod_json = doc["pod"];

    ::baidu::galaxy::sdk::PodDescription& pod = job->pod;
    
    ok = ParsePod(pod_json, &pod);
    return ok;
}

int BuildJobFromConfig(const std::string& conf, ::baidu::galaxy::sdk::JobDescription* job) {
    FILE *fd = fopen(conf.c_str(), "r");
    char buf[5120];
    rapidjson::FileReadStream frs(fd, buf, sizeof(buf));
    rapidjson::Document doc;
    doc.ParseStream<0>(frs);
    if (!doc.IsObject()) {
        fprintf(stderr, "invalid config file\n");
        fclose(fd);
        return -1;
    }
    fclose(fd);
    return ParseDocument(doc, job);
}

} //end namespace client
} //end namespace galaxy
} //end namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */
