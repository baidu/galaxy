/* 
 * File:   container_manager.h
 * Author: haolifei
 *
 * Created on 2016年4月3日, 下午9:04
 */

#pragma once

namespace baidu {
    namespace galaxy {
        namespace agent {
            
            class ContainerManager {
            public:
                boost::shared_ptr<baidu::galaxy::container::Container> CreateContainer() {
                    // check resource
                    // alloc resource
                    // create crgoup
                    // command
                    // process
                }
                
                int ReleaseContainer();
                
                void ListContainer();
            };
        }
    }
}