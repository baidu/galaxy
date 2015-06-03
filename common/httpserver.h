// Copyright (c) 2014, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: sunjunyi01@baidu.com
#ifndef  COMMON_HTTPSERVER_H_
#define  COMMON_HTTPSERVER_H_

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include <vector>
#include "logging.h"
#include "mutex.h"
#include "thread_pool.h"

namespace common {

class HttpFileServer {
public:
    const static off_t TAIL_LIMIT = 8000; //bytes
    HttpFileServer(std::string root_path,int port) : root_path_(root_path),
                                                     port_(port),
                                                     pool_(NULL),
                                                     listen_socket_(0),
                                                     stop_(false) {
        size_t path_len = root_path_.length();
        if (path_len > 0 && root_path_[path_len-1] != '/') {
            root_path_ += "/";
        } 
    }

    ~HttpFileServer() {
        if (listen_socket_ > 0) {
            close(listen_socket_);
        }
        stop_ = true;
        LOG(INFO, "wait threads");
        delete pool_;
        LOG(INFO, "~HttpFileServer()");
    }

    void Start(int thread_num) {
        assert(thread_num > 0);
        pool_ = new ThreadPool(thread_num+1);
        pool_->AddTask(boost::bind(&HttpFileServer::Run, this));
    }

    int Run() {
        fd_set readfd;
        int on;
        sockaddr_in server_addr, client_addr;
        ::signal(SIGPIPE, SIG_IGN);
        listen_socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_socket_ < 0) {
            LOG(WARNING, "create listen socket fail.");
            perror("create socket faild");
            return -1;
        }
        on = 1;
        setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = ::htons(port_);
        server_addr.sin_addr.s_addr = ::inet_addr("0.0.0.0");
        if (::bind(listen_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) {
            LOG(WARNING, "bind to %d faild", port_);
            perror("bind fail");
            return -1;
        }
        if (::listen(listen_socket_, 32) < 0) {
            LOG(WARNING, "listen faild");
            return -1;
        }
        
        while (!stop_) {
            struct timeval timeout={2,0};
            FD_ZERO(&readfd);
            FD_SET(listen_socket_, &readfd);
            ::select(listen_socket_ + 1, &readfd, NULL, NULL, &timeout);
            if (!FD_ISSET(listen_socket_, &readfd)) {
                continue;
            }
            socklen_t sock_len;
            int client_fd = ::accept(listen_socket_, (sockaddr*)&client_addr, &sock_len);
            if (client_fd < 0 ){
                LOG(WARNING, "invalid client fd:%d", client_fd);
                continue;
            }
            pool_->AddTask(boost::bind(&HttpFileServer::HandleClient, this, 
                           client_addr, client_fd));
        }
        return 0;
    }

private:
    struct FileInfo {
        std::string Size;
        std::string LastModified;
    };

    class Session {
    public:
        Session(FILE* in_stream, 
                FILE* out_stream) : in_stream_(in_stream),
                                    out_stream_(out_stream) {
            setlinebuf(in_stream_);
            setlinebuf(out_stream_);
        }
        ~Session() {
            fclose(out_stream_);
            ::shutdown(fileno(out_stream_), SHUT_RDWR);
            fclose(in_stream_);
        }

        bool IsDir(const std::string& path, int *err) {
            struct stat stat_buf;
            int ret = lstat(path.c_str(), &stat_buf);
            *err = ret;
            if (ret == -1) {
                LOG(WARNING, "stat path %s failed err[%d: %s]",
                    path.c_str(),
                    errno,
                    strerror(errno));
            }
            if (S_ISDIR(stat_buf.st_mode)) {
                return true;
            } else {
                return false;
            }
        }

        std::string ReadableFileSize(double size) {
            int i = 0;
            char buf[1024] = {'\0'};
            const char* units[] = {"B", "KB", "MB", "GB", "TB", 
                                   "PB", "EB", "ZB", "YB"};
            while (size > 1024) {
                size /= 1024;
                i++;
            }
            snprintf(buf, sizeof(buf), "%.*f%s", i, size, units[i]);
            return buf;
        }

        bool GetFileInfo(const char* file_name, FileInfo& info) {
            struct stat stat_buf;
            struct tm tm_info;
            char time_buf[1024] = {'\0'};
            int ret = lstat(file_name, &stat_buf);
            if (ret != 0) {
                return false;
            }
            info.Size = ReadableFileSize(stat_buf.st_size);
            time_t modified_time = stat_buf.st_mtime;
            struct tm* l_time = localtime_r(&modified_time, &tm_info);
            if (l_time) {
                strftime(time_buf, sizeof(time_buf),
                        "%Y%m%d %H:%M:%S", l_time);
                info.LastModified = time_buf;
            }
            return true;
        }

        void ShowDir(int file_fd, const std::string& path, int root_len) {
            std::vector<std::string> children;
            std::string out;
            ListDir(path, children);
            std::string parent = path.substr(root_len);
            out += "<h1>Index of " + parent +"</h1>\n";
            out += "<table>\n";
            out += "<tr><th>Name</th><th>Last Modified</th>\n"
                   "<th>Size</th><th>Tail</th></tr>\n";
            out += "<td colspan=\"4\"><hr/></td>\n";
            for (size_t i=0; i < children.size(); i++) {
                std::string tail_button = "";
                std::string href = "";
                std::string anchor = children[i];
                FileInfo info;
                int stat_err = 0;
                std::string full_path = path + "/" + anchor;
                if (anchor == ".") {
                    continue;
                }
                if (parent.size() > 0) {
                    href = parent + "/" + anchor;
                } else {
                    href = anchor ;
                }
                info.Size = "-";
                info.LastModified = "-";
                if (!IsDir(full_path, &stat_err) && stat_err == 0) {
                    tail_button = "<a href=\"/" + href 
                                  + "?tail\"> ... </a>";
                    GetFileInfo(full_path.c_str(), info);
                }
                out += "<tr>\n";
                out += "<td><a href=\"/" + href + "\">" 
                       + anchor + "</a></td>\n";
                out += "<td>" + info.LastModified +"</td>\n";
                out += "<td>" + info.Size + "</td>\n";
                out += "<td>" + tail_button + "</td>\n";
                out += "</tr>\n";
            }
            out += "<td colspan=\"4\"><hr/></td>\n";
            out += "</table>\n";
            fprintf(out_stream_, "HTTP/1.1 200 OK\n");
            fprintf(out_stream_, "Content-Type: text/html\n");
            fprintf(out_stream_, "Content-Length: %lu\n", out.size());
            fprintf(out_stream_, "Server: Galaxy\n");
            fprintf(out_stream_, "Connection: Close\n\n");
            fprintf(out_stream_, "%.*s", static_cast<int>(out.size()), out.data());
            close(file_fd);
        }

        void SendFile(int file_fd, off_t file_size, off_t start_pos){
            off_t transfer_bytes = 0;
            int sock_fd = fileno(out_stream_);
            off_t pos = start_pos;
            fprintf(out_stream_, "HTTP/1.1 200 OK\n");
            fprintf(out_stream_, "Content-Length: %lu\n", file_size - start_pos);
            fprintf(out_stream_, "Server: Galaxy\n");
            fprintf(out_stream_, "Connection: Close\n\n");
            while (transfer_bytes < file_size - start_pos) {
                ssize_t ret = ::sendfile(sock_fd, file_fd, &pos, file_size - transfer_bytes);
                if (ret < 0) {
                    LOG(WARNING, "failed to sendfile");
                    perror("");
                    break;
                }
                transfer_bytes += ret;   
            }
            close(file_fd);
        }

        void Error(const char* msg) {
            LOG(WARNING, "msg: %s", msg);
            fprintf(out_stream_, "%s\n", msg);
        }

        private:
            FILE* in_stream_;
            FILE* out_stream_;
    };

    static void ListDir(const std::string& path, std::vector<std::string>& children ) {
        DIR  *d;
        struct dirent *dir;
        d = opendir(path.c_str());
        if (d) {
            while ((dir = readdir(d)) != NULL)
            {
                children.push_back(dir->d_name);
            }
            closedir(d);
        }
        std::sort(children.begin(), children.end());
    }

    void HandleClient(sockaddr_in /*client_addr*/,
                      int client_fd) {
        char header[256] = {'\0'};
        char method[256] = {'\0'};
        char path[256] = {'\0'};
        char version[256] = {'\0'};
        FILE* in_stream = ::fdopen(client_fd, "r");
        FILE* out_stream = ::fdopen(dup(client_fd), "w");
        if (!in_stream || !out_stream) {
            perror("call fdopen failed");
            close(client_fd);
            return;
        }
        Session session(in_stream, out_stream); 
        fgets(header, sizeof(header), in_stream);
        if (sscanf(header, "%s %s %s", method, path, version) == 3){
            std::string real_path;
            std::string http_path(path);
            bool tail_only = false;
            std::size_t found = http_path.rfind("?tail");
            if (found != std::string::npos) {
                tail_only = true;
                http_path.erase(found);
            }
            real_path = root_path_ + http_path.substr(1);
            if (::access(real_path.c_str(), F_OK) == 0) {
                int file_fd = ::open(real_path.c_str(), O_RDONLY);
                struct stat stat_buf;
                fstat(file_fd, &stat_buf);
                if (S_ISDIR(stat_buf.st_mode)) {
                    session.ShowDir(file_fd, real_path, root_path_.size());
                    return ;
                }
                off_t file_size = stat_buf.st_size;
                off_t start_pos = 0;
                if (tail_only && file_size > TAIL_LIMIT) {
                    start_pos = (file_size - TAIL_LIMIT);
                }
                session.SendFile(file_fd, file_size, start_pos);
            } else {
                LOG(WARNING, "%s can not be found", real_path.c_str());
                session.Error("file not exists\n");
            }
        } else {
            LOG(WARNING, "bad header: %s", header);
            session.Error("bad method");
        }
    }
private:
    std::string root_path_;
    int port_;
    ThreadPool* pool_;
    int listen_socket_;
    bool stop_;
};

} //namespace common

#endif  //COMMON_HTTPSERVER_H_
