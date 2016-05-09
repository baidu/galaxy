// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include <string>
#include <sstream>

namespace baidu {
    namespace galaxy {
        namespace util {

            class ErrorCode {
            public:

                ErrorCode() : code_(-1) {

                }

                ErrorCode(int code, const std::string& msg) :
                code_(code),
                message_(msg) {
                }

                ErrorCode(const char* file, const char* function, uint32_t line, int code, const char* format, ...) : code_(code) {
                    assert(NULL != file);
                    assert(NULL != function);
                    assert(NULL != format);

                    std::stringstream ss;
                    ss << "[" << file << ":" << function << ":" << line << "] ";
                    char buf[2048];
                    va_list va;
                    va_start(va, format);
                    vsnprintf(buf, sizeof buf, format, va);
                    va_end(va);
                    ss << buf;
                    message_ = ss.str();
                }

                ErrorCode(const char* file,
                        const char* function,
                        uint32_t line,
                        int code,
                        int errnum,
                        const char* format) : code_(code) {

                    assert(NULL != file);
                    assert(NULL != function);
                    assert(NULL != format);

                    std::stringstream ss;
                    ss << "[" << file << ":" << function << ":" << line << "] ";
                    char buf[2048];
                    va_list va;
                    va_start(va, format);
                    vsnprintf(buf, sizeof buf, format, va);
                    va_end(va);
                    ss << buf << ": " << strerror(errnum);
                    message_ = ss.str();
                }

                int Code() const {
                    return code_;
                }

                const std::string Message() const {
                    return message_;
                }


            private:
                int code_;
                std::string message_;
            };
        }
    }
}

#define SET_ERRORCODE_MSG(obj, format, args...) obj.SetMessage(__FILE__, __FUNCTION__, __LINE__, format, ##args);
#define ERRORCODE(code, format, args...) baidu::galaxy::util::ErrorCode(__FILE__, __FUNCTION__, __LINE__, code, format, ##args)
#define PERRORCODE(code, errnum, format, args...) baidu::galaxy::util::ErrorCode(__FILE__, __FUNCTION__, __LINE__, code, format, ##args)
#define ERRORCODE_OK baidu::galaxy::util::ErrorCode(0, "")
