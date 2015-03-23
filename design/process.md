## Linux Process 调研

### 层级关系

```
session --
         | 
          \group1 --
                   |
                    \process1 --
                   |            |
                    \process2    \ thread 1
                                |
                                 \ thread 2
```
每个对象都有自己的id sid,gid,pid,tid 
### 创建一个进程
当创建一个进程时，如果不设置gid，子进程默认使用父进程gid

### 一个问题，如何方便的结束一个process tree
当运行一个task时，task可能会创建复杂结构的process tree, 如果一个一个process去kill很麻烦，但是使用process group去kill却非常方便
* fork 一个leader process
* 在子进程里面设置gid
* killpg
样例代码
```
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
int main ()
{
    //fork出来pid不会和存在的的process group id相同
    pid_t pid = fork();
    if(pid==0){
        pid_t my_pid = getpid()
        setpgid(pid,pid);
        FILE *fd = popen("python -m SimpleHTTPServer 12345","r");
        pclose(fd);
        return 0;
    }else{
        killpg(pid,9);
    }   
    return 0;

}
```

### reference
[processes](http://www.win.tue.nl/~aeb/linux/lk/lk-10.html)
                  
