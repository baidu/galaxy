/* 
 * File:   resource.h
 * Author: haolifei
 *
 * Created on 2016年4月3日, 下午9:13
 */

#pragma once
#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

namespace baidu{
    namespace galaxy {
        namespace resource {
            class Resource {
                
            public:
                void Allocate(int64_t resource);
                void Release(int64_t resource);
                void* LockResource(int64_t resource);
                void UnlockResource(void* handle);
                void Allocate(void* handle);
                
                int64_t TotalResource();
                RemainResource();
                
            };
        } 
    }
}