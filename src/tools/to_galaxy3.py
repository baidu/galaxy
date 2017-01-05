import json
import os
import sys
from collections import OrderedDict

def read_json_from_file(path):
    file = open(path, "r")
    con = file.read()
    return json.loads(con, object_pairs_hook=OrderedDict)


def write_json_to_file(dt, path):
    with open(path, "wb") as f:
        f.write(json.dumps(dt, indent = 1, ensure_ascii=False, sort_keys=True).encode('utf-8'))

def create_default_task() :
    task = {}
    blkio = {}
    blkio["weight"] = 500
    task["blkio"] = blkio

    tcp_throt = {}
    tcp_throt = {}
    tcp_throt["recv_bps_quota"] = "300M"
    tcp_throt["send_bps_excess"] = True
    tcp_throt["send_bps_quota"] = "300M"
    tcp_throt["recv_bps_excess"] = True
    task["tcp_throt"] = tcp_throt

    cpu = {}
    cpu["milli_core"] = 1000
    cpu["excess"] = False
    task["cpu"] = cpu

    memory = {}
    memory["size"] = "800M"
    memory["excess"] = False
    task["memory"] = memory

    exe_package = {}
    exe_package["stop_cmd"] = ""
    exe_package["start_cmd"] = ""
    package = {}
    package["version"] = "1.0.0"
    package["source_path"] = "ftp://xxx"
    package["dest_path"] = "/home/galaxy"
    exe_package["package"] = package
    task["exe_package"] = exe_package
    return task

def default_g3_json():
    job={}
    job["name"] = "example"
    job["run_user"] = "galaxy"
    job["priority"] = 100
    job["v2_support"] = False
    job["volum_view"] = "kVolumViewTypeEmpty"

    deploy={}
    deploy["max_per_host"] = 1
    deploy["pools"] = "test"
    deploy["step"] = 1
    deploy["replica"] = 1
    deploy["interval"] = 1
    job["deploy"] = deploy

    pod = {}
    workspace_volum = {}
    workspace_volum["exclusive"] = False
    workspace_volum["medium"] = "kDisk"
    workspace_volum["readonly"] = False
    workspace_volum["dest_path"] = "/home/galaxy"
    workspace_volum["type"] = "kEmptyDir"
    workspace_volum["use_symlink"] = False
    workspace_volum["size"] = "30G"
    pod["workspace_volum"] = workspace_volum

    pod["tasks"] = []
    job["pod"] = pod
    return job

def set_volume_container(job, vc):
    job["volum_jobs"] = vc

def set_pool(job, pool):
    job["deploy"]["pool"] = pool

def add_port(job, name, value):
    tasks = job["tasks"]
    tasks[0]["ports"] = []
    port = {}
    port["port_name"] = name
    port["port"] = value
    tasks[0]["ports"].append(port)

def add_data_volum(job, size, medium, pos):
    v = {}
    v["size"] = size
    v["medium"] = medium 
    v["dest_path"] = pos
    v["exclusive"] = False
    v["use_symlink"] = False
    v["type"] = "kEmptyDir"
    
    if not "data_volums" in job["pod"]:
        job["pod"]["data_volums"] = []
    job["pod"]["data_volums"].append(v)


def add_port(job, name, port):
    return False
    

def convert(g2, g3):
    if not "name" in g2:
        print("name filed is required")
        return False
    g3["name"] = g2["name"]

   
    if not "pod" in g2:
        print("pod is required in g2")
        return False
    g2_pod = g2["pod"]

    #tasks
    if "tasks" not in g2_pod:
        print("tasks is requred in g2.pod")
        return False

    if len(g2_pod["tasks"]) < 1:
        print("g2 task size is 0")
        return False

    version="1.0.0"
    if "version" in g2["pod"]:
        version = g2["pod"]["version"]

    for g2_task in g2_pod["tasks"]:
        g3_task = create_default_task()
        if "start_command" in g2_task:
            g3_task["exe_package"]["start_cmd"] = g2_task["start_command"]

        if "stop_command" in g2_task:
            g3_task["exe_package"]["stop_cmd"] = g2_task["stop_command"]

        g3_task["exe_package"]["package"]["version"] = version

        if not "binary" in g2_task:
            print("binary is requred in g2.tasks")
            return False
        g3_task["exe_package"]["package"]["source_path"] = g2_task["binary"]

        if "cpu_isolation_type" in g2_task and g2_task["cpu_isolation_type"] == "kCpuIsolationSoft":
            g3_task["cpu"]["excess"] = True

        if not "requirement" in g2_task:
            print("requirement is required in g2.tasks")
            return false

        if not "millicores" in g2_task["requirement"]:
            print("millicore is requred in g2.tasks.requirement")
            return False
        g3_task["cpu"]["size"] = g2_task["requirement"]["millicores"]

        if not "memory" in g2_task["requirement"]:
            print("memory is required in g2.tasks.requirement")
            return false
        g3_task["memory"]["size"] = g2_task["requirement"]["memory"]
        g3["pod"]["tasks"].append(g3_task)

    if not "replica" in g2:
        print("replica is required in g2")
        return False
    g3["deploy"]["replica"] = g2["replica"] 

    #type 
    if not "deploy_step" in g2:
        print("deploy step is requered in g2")
        return False
    g3["deploy"]["step"] = g2["deploy_step"]

    if not "requirement" in g2_pod:
        print "requirement is required in g2.pod"
        return False
    if "tmpfs" in g2_pod:
        size = g2_pod["tmpfs"]["size"]
        path = g2_pod["tmpfs"]["path"]
        add_data_volum(size, "kTmpfs", path)

    #port
    if "ports" in g2_pod:
        ports = g2_pod["ports"]
        for port in ports:
            add_port(g3, str(port), str(port))
    return True

def print_help(arg0):
    print("usage: %s g2_json g3_json" % arg0)

if "__main__" == __name__:
    if len(sys.argv) < 3:
        print_help(sys.argv[0])
        exit(1)

    g2_json = sys.argv[1]
    g3_json = sys.argv[2]

    g3 = default_g3_json()
    g2 = read_json_from_file(g2_json)
    
    if not convert(g2, g3):
        print("convert falied")

    write_json_to_file(g3, g3_json)
