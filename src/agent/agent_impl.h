#ifndef _BAIDU_GALAXY_AGENT_IMPL_H_
#define _BAIDU_GALAXY_AGENT_IMPL_H_

#include <string>
#include <map>
#include <set>

#include "src/protocol/agent.pb.h"

namespace baidu {
namespace galaxy {

class AgentImpl : public Agent {
public:

AgentImpl();
virtual ~AgentImpl();

void CreateContainer(::google::protobuf::RpcController* controller,
                     const ::baidu::galaxy::CreateContainerRequest* request,
                     ::baidu::galaxy::CreateContainerResponse* response,
                     ::google::protobuf::Closure* done);

void RemoveContainer(::google::protobuf::RpcController* controller,
                     const ::baidu::galaxy::RemoveContainerRequest* request,
                     ::baidu::galaxy::RemoveContainerResponse* response,
                     ::google::protobuf::Closure* done);

void ListContainers(::google::protobuf::RpcController* controller,
                    const ::baidu::galaxy::ListContainersRequest* request,
                    ::baidu::galaxy::ListContainersResponse* response,
                    ::google::protobuf::Closure* done);

void Query(::google::protobuf::RpcController* controller,
           const ::baidu::galaxy::QueryRequest* request,
           ::baidu::galaxy::QueryResponse* response,
           ::google::protobuf::Closure* done);



};

}
}

#endif

