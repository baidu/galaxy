/* 
 * File:   command_line.h
 * Author: haolifei
 *
 * Created on 2016年4月5日, 下午9:55
 */

#pragma once

namespace baidu {
    namespace galaxy {
        namespace container {
            
            class CommandLine {
            public:
                CommandLine* SetId(const std::string& id);
                CommandLine* SetUser(const std::string& user);
                CommandLine* Append(const std::string& item);
                std::string Str();
            private:
                std::string _cmd;
            };
        }
    }
}