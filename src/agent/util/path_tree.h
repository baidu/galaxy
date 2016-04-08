/* 
 * File:   path_tree.h
 * Author: haolifei
 *
 * Created on 2016年4月5日, 上午10:55
 */

/*
 * 
 * root/
 * |-- gc_dir
 * `-- work_dir
 *  |-- container1
 *  |   |-- app
 *  |   |-- bin
 *  |   |-- etc
 *  |   `-- home
 *  `-- container2
 */


#pragma once
#include <string>

namespace baidu {
    namespace galaxy {
        namespace util {
            class PathTree {
            public:
                PathTree(const std::string& root_path);
                ~PathTree();
                
                const std::string RootPath();
                const std::string GcDir();
                const std::string WorkDir();
                const std::string ContainerDir(const std::string& container);
                const std::string AppRootDir();
            private:
            };
        }
    }
}