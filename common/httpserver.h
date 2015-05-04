// Copyright (c) 2014, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: sunjunyi01@baidu.com
#ifndef  COMMON_HTTPSERVER_H_
#define  COMMON_HTTPSERVER_H_

#include <arpa/inet.h>
#include <dirent.h>
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
    HttpFileServer(std::string root_path,int port) : root_path_(root_path),
                                                     port_(port),
                                                     pool_(NULL),
                                                     listen_socket_(0),
                                                     stop_(false) {

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

        void ShowDir(int file_fd, const std::string& path, int root_len) {
            std::vector<std::string> children;
            std::string out;
            ListDir(path, children);
            out += "<ul>";
            std::string parent = path.substr(root_len);
            for (size_t i=0; i < children.size(); i++) {
                if (parent.size() > 0) {
                    out += "<li><a href=\"/" + parent + "/" + children[i]+"\">" + children[i] + "</a></li>\n";
                } else {
                    out += "<li><a href=\"/" + children[i] + "\">" + children[i] + "</a></li>\n";
                }
            }
            out += "</ul>";
            fprintf(out_stream_, "HTTP/1.1 200 OK\n");
            fprintf(out_stream_, "Content-Type: text/html\n");
            fprintf(out_stream_, "Content-Length: %lu\n", out.size());
            fprintf(out_stream_, "Server: Galaxy\n");
            fprintf(out_stream_, "Connection: Close\n\n");
            fprintf(out_stream_, "%.*s", static_cast<int>(out.size()), out.data());
            close(file_fd);
        }
        void SendFile(int file_fd, off_t file_size){
            off_t transfer_bytes = 0;
            int sock_fd = fileno(out_stream_);
            off_t pos = 0;
            fprintf(out_stream_, "HTTP/1.1 200 OK\n");
            fprintf(out_stream_, "Content-Length: %lu\n", file_size);
            fprintf(out_stream_, "Server: Galaxy\n");
            fprintf(out_stream_, "Connection: Close\n\n");
            while (transfer_bytes < file_size) {
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
                session.SendFile(file_fd, file_size);
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

