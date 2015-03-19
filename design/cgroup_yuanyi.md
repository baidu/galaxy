Available Subsystems in Red Hat Enterprise Linux
• blkio — this subsystem sets limits on input/output access to and from block devices such as physical drives (disk, solid state, USB, etc.).
• cpu — this subsystem uses the scheduler to provide cgroup tasks access to the CPU.
• cpuacct — this subsystem generates automatic reports on CPU resources used by tasks in a cgroup.
• cpuset — this subsystem assigns individual CPUs (on a multicore system) and memory nodes to tasks in a cgroup.
• devices — this subsystem allows or denies access to devices by tasks in a cgroup.
• freezer — this subsystem suspends or resumes tasks in a cgroup.
• memory — this subsystem sets limits on memory use by tasks in a cgroup, and generates automatic reports on memory resources used by those tasks.
• net_cls — this subsystem tags network packets with a class identifier (classid) that allows the Linux traffic controller (tc) to identify packets originating from a particular cgroup task.
• net_prio — this subsystem provides a way to dynamically set the priority of network traffic per network interface.
• ns — the namespace subsystem.</p>
Pasted from <a href="https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Resource_Management_Guide/ch01.html">https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Resource_Management_Guide/ch01.html</a>
在redhat官网上描述原文，the relationship between subsystem， control group， hierarchy and task
R1
A single hierarchy can have one or more than one subsystems attached to it
As a consequence, the cpu and memory subsystems (or any number of subsystems) can be attached to a single hierarchy, as long as each one is not attached to any other hierarchy which has any other subsystems attached to it already (see Rule 2).</p>
R2
Any single subsystem (such as cpu) cannot be attached to more than one hierarchy if one of those hierarchies has a different subsystem attached to it already.
As a consequence, the cpu subsystem can never be attached to two different hierarchies if one of those hierarchies already has the memory subsystem attached to it. However, a single subsystem can be attached to two hierarchies if both of those hierarchies have only that subsystem attached.
R3
Each time a new hierarchy is created on the systems, all tasks on the system are initially members of the default cgroup of that hierarchy, which is known as the root cgroup. For any single hierarchy you create, each task on the system can be a member of exactly one cgroup in that hierarchy. A single task may be in multiple cgroups, as long as each of those cgroups is in a different hierarchy. As soon as a task becomes a member of a second cgroup in the same hierarchy, it is removed from the first cgroup in that hierarchy. At no time is a task ever in two different cgroups in the same hierarchy.
As a consequence, if the cpu and memory subsystems are attached to a hierarchy named cpu_mem_cg, and the net_cls subsystem is attached to a hierarchy named net, then a running httpdprocess could be a member of any one cgroup in cpu_mem_cg, and any one cgroup in net.
The cgroup in cpu_mem_cg that the httpd process is a member of might restrict its CPU time to half of that allotted to other processes, and limit its memory usage to a maximum of 1024 MB. Additionally, the cgroup in net that it is a member of might limit its transmission rate to 30 megabytes per second.
When the first hierarchy is created, every task on the system is a member of at least one cgroup: the root cgroup. When using cgroups, therefore, every system task is always in at least one cgroup.
R4
Any process (task) on the system which forks itself creates a child task. A child task automatically inherits the cgroup membership of its parent but can be moved to different cgroups as needed. Once forked, the parent and child processes are completely independent.
As a consequence, consider the httpd task that is a member of the cgroup named half_cpu_1gb_max in the cpu_and_mem hierarchy, and a member of the cgroup trans_rate_30 in the nethierarchy. When that httpd process forks itself, its child process automatically becomes a member of the half_cpu_1gb_max cgroup, and the trans_rate_30 cgroup. It inherits the exact same cgroups its parent task belongs to.
From that point forward, the parent and child tasks are completely independent of each other: changing the cgroups that one task belongs to does not affect the other. Neither will changing cgroups of a parent task affect any of its grandchildren in any way. To summarize: any child task always initially inherit memberships to the exact same cgroups as their parent task, but those memberships can be changed or removed later.
根据对上面的理解：
    1. 每个层级可以添加多个subsystem，也就是在mount -t cgroup none /cgroup，其中挂载cgroup文件系统到/cgroup目录下，且未指定特殊subsystem，会attach所有subsystem，操作以上命令后，观察/cgroup下，出现所有subsytem配置文件
    2. 这里面所指的层级都是root层级，cpu子系统可以attach上只有cpu子系统attach的层级
    3. task可以同时存在于多个cgroup中，但是cgroup必须保证在不同的层级
    4. fork的子进程可以随意变更cgroup

综合以上理解，不同subsystem可以被mount到不同的层级中，如果某个层级包含多个subsystem，则这些subsystem无法mount到其他层级，每次mount的层级为root层级，包含所有当前系统运行的task（根据/cgroup/tasks），可以在同一层级下创建目录的方式创建子cgroup，子cgroup共享父cgroup的资源限制，并做相关调整。且通过实验，root层级无法更改配额。</p>
概念：
    1. Task: 进程的概念
    2. Control group：按照一定资源配置的进程组，属于该cgroup的进程可以使用该cgroup中分配的资源
    3. Hierarchy：多个control group，可以组织成层级的关系（树），子cgroup集成父cgroup的特性（如第一层级，控制所有cpu以及mem使用，而在该group中的进程分成a,b两组，每组分到不同的子cgroups中，且不同cgroups里面对bio的控制不同）
    4. subsystem：子系统为最基本的资源控制器。

个人理解：
 subsystem是实际进行资源控制的，资源控制器只会属于一个cgroup，而cgroup中包含的进程组均享受相同的资源控制，而cgroup同样包含父子层级关系，对于第一个创建的cgroup为rootcgroup，子层级共享父层级的资源限制。
子系统通过配置对属于指定cgroup的进程进行资源限制.
