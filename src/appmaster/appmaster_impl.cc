#include "appmaster_impl.h"
#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/utsname.h>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <snappy.h>

namespace baidu {
namespace galaxy {

AppMasterImpl::AppMasterImpl() {

}

AppMasterImpl::~AppMasterImpl() {

}

void AppMasterImpl::AssignTask(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::AssignTaskRequest* request,
                              ::baidu::galaxy::AssignTaskResponse* response,
                              ::google::protobuf::Closure* done) {

}


void AppMasterImpl::ListTasks(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::ListTasksRequest* request,
                              ::baidu::galaxy::ListTasksResponse* response,
                              ::google::protobuf::Closure* done) {

}

void AppMasterImpl::StartTask(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::StartTaskRequest* request,
                              ::baidu::galaxy::StartTaskResponse* response,
                              ::google::protobuf::Closure* done) {

}


void AppMasterImpl::StopTask(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::StopTaskRequest* request,
                             ::baidu::galaxy::StopTaskResponse* response,
                             ::google::protobuf::Closure* done) {

}


void AppMasterImpl::UpdateTask(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::UpdateTaskRequest* request,
                               ::baidu::galaxy::UpdateTaskResponse* response,
                               ::google::protobuf::Closure* done) {

}


void AppMasterImpl::ExecuteCmd(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::ExecuteCmdRequest* request,
                               ::baidu::galaxy::ExecuteCmdResponse* response,
                               ::google::protobuf::Closure* done) {

}


}
}


