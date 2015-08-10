// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_COLLECTIONS_H
#define BAIDU_GALAXY_COLLECTIONS_H
#include <stdlib.h>
#include <algorithm>
#include <vector>
namespace baidu {
namespace galaxy {

class Collections {

public:

    template<class T>
    static void Shuffle(std::vector<T>& list) {
        for (size_t i = list.size(); i > 1; i--) {
            T tmp = list[i-1];
            size_t target_index = ::rand() % i ;
            list[i-1] = list[target_index];
            list[target_index] = tmp;
        }
    }
};


}// end of galaxy
}// end of baidu
#endif
