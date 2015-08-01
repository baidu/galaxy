#include "agent_impl.h"

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() {

}

AgentImpl::~AgentImpl() {

}

void AgentImpl::Query(::google::protobuf::RpcController* controller, 
                      const ::baidu::galaxy::QueryRequest* request,
                      ::baidu::galaxy::QueryResponse* response,
                      ::google::protobuf::Closure* done) {

}

void AgentImpl::RunPod(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::RunPodRequest* request,
                       ::baidu::galaxy::RunPodResponse* response,
                       ::google::protobuf::Closure* done) {
}

void AgentImpl::KillPod(::google::protobuf::RpcController* controller,
                        const ::baidu::galaxy::KillPodRequest* request,
                        ::baidu::galaxy::KillPodResponse* response, 
                        ::google::protobuf::Closure* done) {

}


} // ending namespace galaxy
} // ending namespace baidu
