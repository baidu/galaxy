## Cgroup Memory 子系统使用以及配置文件介绍

### 简单使用

#### 内核版本
3.10.0-123.4.2.el7.x86_64

#### 创建一个memory group
创建之前确保已经mount cgroup
`mkdir /sys/fs/cgroup/memory/0
echo $$ > /sys/fs/cgroup/memory/0/tasks
echo 4M > /sys/fs/cgroup/memory/0/memory.limit_in_bytes##G,M
cat /sys/fs/cgroup/memory/0/memory.limit_in_bytes
4194304`

### 重要配置文件介绍

#### task 文件
这个文件包含需要隔离的进程,线程id,属性为 *读写* ,所以如果要将一个进程加入到一个cgroup下面可以修改这个文件,
当然系统也会改变这个文件，比如一个进程在运行中 *创建线程* 或者 *fork进程* 都会出现task文件中

#### cgroup.procs 文件
这个文件包含需要隔离的进程id,在低版本内核上面是属性为 *只读* ,例如2.6.32_1-15-0-0,但是在高版本内核上面是
属性是 *读写*, 在当前基于高版本内核的 [mesos](http://mesos.apache.org/) ,[libcontainer](https://github.com/docker/libcontainer) 都是修改这个文件，所以在低版本内核使用它们会有问题

#### memory.limit_in_bytes文件
这个文件显示当前cgroup最大能够使用的内存大小,文件属性为 *读写*,可以通过修改此文件动态调整容器内存限制，当应用达到内存限制的时候，会出现[OOM]()

#### memory.soft_limit_in_bytes文件
和 memory.limit_in_bytes区别就是，应用可以超出memory.soft_limit_in_bytes，和其他应用共享一部分内存。但是当系统资源紧张的时候，cgroup会把应用的内存降低到soft_limit_in_bytes这个参数

#### memory.stat 文件
当需要监控内存使用情况的时候可以使用这个文件，比如计算当前内存使用情况 mem_used=rss+cache(+swap),memory.usage_in_bytes也是内存使用情况，但是这个不准确，具体见[memory controller](https://www.kernel.org/doc/Documentation/cgroups/memory.txt)

#### OOM Control
TODO
#### cgroup.event_control 文件
事件通知文件, 在memory子系统有个功能就是设置一个内存阀值，当应用内存达到这个值，可以从这个文件获取事件通知















