#include "agent_impl.h"
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

AgentImpl::AgentImpl() {

}

AgentImpl::~AgentImpl() {

}

void AgentImpl::CreateContainer(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::CreateContainerRequest* request,
                                ::baidu::galaxy::CreateContainerResponse* response,
                                ::google::protobuf::Closure* done) {

}


void AgentImpl::RemoveContainer(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::RemoveContainerRequest* request,
                                ::baidu::galaxy::RemoveContainerResponse* response,
                                ::google::protobuf::Closure* done) {

}


void AgentImpl::ListContainers(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::ListContainersRequest* request,
                               ::baidu::galaxy::ListContainersResponse* response,
                               ::google::protobuf::Closure* done) {

}


void AgentImpl::Query(::google::protobuf::RpcController* controller,
                      const ::baidu::galaxy::QueryRequest* request,
                      ::baidu::galaxy::QueryResponse* response,
                      ::google::protobuf::Closure* done) {

}


}
}


