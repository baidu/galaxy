// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <mapreduce/mapreduce.h>

#include <ctype.h>
#include <string>
#include <sstream>

using std::string;

int StringToInt(const std::string& s) {
    int ret = 0;
    std::istringstream is(s);
    is >> ret;
    return ret;
}

std::string IntToString(int v) {
    char buf[16];
    snprintf(buf, 16, "%d", v);
    return buf;
}

// Users map function
class WordCounter : public Mapper {
public:
    virtual void Map(const MapInput& input) {
        const string& text = input.value();
        const int n = text.size();
        for (int i = 0; i < n; ) {
            // Skip past leading whitespace
            while ((i < n) && isspace(text[i]))
                i++;
            
            // Find word end
            int start = i;
            while ((i < n) && !isspace(text[i]))
                i++;
            if (start < i)
                Emit(text.substr(start,i-start),1);
        }
    }
};

REGISTER_MAPPER(WordCounter);

// Users reduce function
class Adder : public Reducer {
    virtual void Reduce(ReduceInput* input) {
        // Iterate over all entries with the
        // same key and add the values
        int64_t value = 0;
        while (!input->done()) {
            value += StringToInt(input->value());
            input->NextValue();
        }
        
        // Emit sum for input->key()
        Emit(IntToString(value));
    }
};

REGISTER_REDUCER(Adder);

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
