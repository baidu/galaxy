// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <cstring>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>

namespace baidu {
namespace galaxy {
namespace client {

std::string FormatDate(uint64_t datetime) {
    if (datetime < 100) {
        return "-";
    }

    char buf[100];
    time_t time = datetime / 1000000;
    struct tm *tmp;
    tmp = localtime(&time);
    strftime(buf, 100, "%F %X", tmp);
    std::string ret(buf);
    return ret;
}

bool GetHostname(std::string* hostname) {
    char buf[100];
    if (gethostname(buf, 100) != 0) {
        fprintf(stderr, "gethostname failed\n");
        return false;
    }
    *hostname = buf;
    return true;
}

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

//读取endpoint
bool LoadAgentEndpointsFromFile(const std::string& file_name, std::vector<std::string>* agents) {
    const int LINE_BUF_SIZE = 1024;
    char line_buf[LINE_BUF_SIZE];
    std::ifstream fin(file_name.c_str(), std::ifstream::in);        
    if (!fin.is_open()) {
        fprintf(stderr, "open %s failed\n", file_name.c_str());
        return false; 
    }
    
    bool ret = true;
    int i = 0;
    while (!fin.eof() && fin.good()) {
        fin.getline(line_buf, LINE_BUF_SIZE);     
        if (fin.gcount() == LINE_BUF_SIZE) {
            fprintf(stderr, "line buffer size overflow\n");     
            ret = false;
            break;
        } else if (fin.gcount() == 0 || strlen(line_buf) == 0) {
            continue; 
        }
        fprintf(stdout, "label %ld: %s\n", fin.gcount(), line_buf);
        // NOTE string size should == strlen
        std::string agent_endpoint(line_buf, strlen(line_buf));
        agents->push_back(agent_endpoint);
        ++i;
    }

    if (!ret) {
        fin.close();
        return false;
    }

    fin.close(); 
    return true;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu
/* vim: set ts=4 sw=4 sts=4 tw=100 */
