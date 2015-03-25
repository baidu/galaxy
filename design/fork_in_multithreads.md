## Use fork in thread ##

### what fork do ###
通过调用fork接口，创建子进程，且此时子进程拷贝父进程资源（当然不是立即拷贝，进行copy_on_write）.
通过系统 man fork 的描述，子进程与父进程的区别只有在pid，ppid不同，以及资源使用，文件锁，待处理信号量被清空.
[fork()](http://linux.die.net/man/2/fork)，更加详细的列出了child进程与父进程的区别.
其中包含:
- child包含唯一的pid，并且该pid对应的gpid也是唯一的.（默认child pgid与父进程一致，可通过setpgid，将child进程以
及后续的grandchild进程整体分组管理， 如通过killpg给指定进程组发送signal）
- child不会进程内存锁（mlock/mlockall）. mlock/mlockall主要是负责内存page级别的锁
- child会清零资源使用统计以及cpu计数
- child会清空pending signal queue
- child不会继承semaphore
- child不会进程alarm等计时数据
- 对于通过接口madvise接口标记MADV_DONTFORK的内存page，不会被继承.(通过该标记，可以指定内存地址以及length，来保证该内存不会被fork继承)
- child不会继承标准的异步io操作
- child不会继承fcntl record级别的锁
- child不会继承目录改变触发机制（F_NOTIFY fcntl，该标记可通过发现文件目录改变或目录下的文件改变）

### what will happen ###
 - **锁状态的复制**,如果在多线程下，通过一个线程去调用fork，则child会复制整个进程的内存资源，这样对于其他线程的任何内存操作，原子区操作
 的状态均会被子进程拷贝，如pthread库中的mutex，condition，以及malloc，printf等glibc的接口中锁元语的状态.对于这类接口的描述为async-not-safe
 接口，如果对于此类接口正在调用过程中，其他线程发生fork调用，且子进程同时调用了该接口，则会导致子进程被死锁的风险.
 - **句柄资源的复制**,fork后子进程会复制父进程打开的所有句柄，导致父进程中止后，子进程占用socket句柄，父进程无法恢复

### how to deal with it ###
- 对于句柄资源，应该在open句柄的时候设置O_CLOEXEC，或者通过fcntl设置FD_CLOEXEC参数.由于galaxy项目中依赖外部pbrpc，无法获取socket句柄，
暂时通过扫描proc下的句柄记录解决
- 对于锁状态的复制，由于在线程中fork子进程的需求主要是通过fork后调用exec函数去执行其他程序，且exec会覆盖并清理掉原有进程资源，因此应该
尽量在子进程中减少fork与exec间的工作，且保证所有执行接口均为async-safe(此处可参考[async-signal-safe](http://man7.org/linux/man-pages/man7/signal.7.html))

参考文章： 
  - [Threads and fork(): think twice before mixing them.](http://www.linuxprogrammingblog.com/threads-and-fork-think-twice-before-using-them)
  - [Signal](http://man7.org/linux/man-pages/man7/signal.7.html)
  - [fork](http://linux.die.net/man/2/fork)
