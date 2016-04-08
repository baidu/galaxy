#ifndef _BAIDU_GALAXY_RESMAN_IMPL_H_
#define _BAIDU_GALAXY_RESMAN_IMPL_H_

#include <string>
#include <map>
#include <set>

#include "src/protocol/resman.pb.h"

namespace baidu {
namespace galaxy {

class ResManImpl : public ResMan {
public:

ResManImpl();
~ResManImpl();

void EnterSafeMode(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::EnterSafeModeRequest* request,
                   ::baidu::galaxy::EnterSafeModeResponse* response,
                   ::google::protobuf::Closure* done);

void LeaveSafeMode(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::LeaveSafeModeRequest* request,
                   ::baidu::galaxy::LeaveSafeModeResponse* response,
                   ::google::protobuf::Closure* done);

void Status(::google::protobuf::RpcController* controller,
            const ::baidu::galaxy::StatusRequest* request,
            ::baidu::galaxy::StatusResponse* response,
            ::google::protobuf::Closure* done);

void KeepAlive(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::KeepAliveRequest* request,
               ::baidu::galaxy::KeepAliveResponse* response,
               ::google::protobuf::Closure* done);

void SubmitJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::SubmitJobRequest* request,
               ::baidu::galaxy::SubmitJobResponse* response,
               ::google::protobuf::Closure* done);

void UpdateJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::UpdateJobRequest* request,
               ::baidu::galaxy::UpdateJobResponse* response,
               ::google::protobuf::Closure* done);

void RemoveJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::RemoveJobRequest* request,
               ::baidu::galaxy::RemoveJobResponse* response,
               ::google::protobuf::Closure* done);

void AddAgent(::google::protobuf::RpcController* controller,
              const ::baidu::galaxy::AddAgentRequest* request,
              ::baidu::galaxy::AddAgentResponse* response,
              ::google::protobuf::Closure* done);

void RemoveAgent(::google::protobuf::RpcController* controller,
                 const ::baidu::galaxy::RemoveAgentRequest* request,
                 ::baidu::galaxy::RemoveAgentResponse* response,
                 ::google::protobuf::Closure* done);

void OnlineAgent(::google::protobuf::RpcController* controller,
                 const ::baidu::galaxy::OnlineAgentRequest* request,
                 ::baidu::galaxy::OnlineAgentResponse* response,
                 ::google::protobuf::Closure* done);

void OfflineAgent(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::OfflineAgentRequest* request,
                  ::baidu::galaxy::OfflineAgentResponse* response,
                  ::google::protobuf::Closure* done);

void ListAgents(::google::protobuf::RpcController* controller,
                const ::baidu::galaxy::ListAgentsRequest* request,
                ::baidu::galaxy::ListAgentsResponse* response,
                ::google::protobuf::Closure* done);

void CreateTag(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::CreateTagRequest* request,
               ::baidu::galaxy::CreateTagResponse* response,
               ::google::protobuf::Closure* done);

void RemoveTag(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::RemoveTagRequest* request,
               ::baidu::galaxy::RemoveTagResponse* response,
               ::google::protobuf::Closure* done);

void ListTags(::google::protobuf::RpcController* controller,
              const ::baidu::galaxy::ListTagsRequest* request,
              ::baidu::galaxy::ListTagsResponse* response,
              ::google::protobuf::Closure* done);

void AddAgentToTag(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::AddAgentToTagRequest* request,
                   ::baidu::galaxy::AddAgentToTagResponse* response,
                   ::google::protobuf::Closure* done);

void RemoveAgentFromTag(::google::protobuf::RpcController* controller,
                        const ::baidu::galaxy::RemoveAgentFromTagRequest* request,
                        ::baidu::galaxy::RemoveAgentFromTagResponse* response,
                        ::google::protobuf::Closure* done);

void ListAgentsByTag(::google::protobuf::RpcController* controller,
                     const ::baidu::galaxy::ListAgentsByTagRequest* request,
                     ::baidu::galaxy::ListAgentsByTagResponse* response,
                     ::google::protobuf::Closure* done);

void AddUser(::google::protobuf::RpcController* controller,
             const ::baidu::galaxy::AddUserRequest* request,
             ::baidu::galaxy::AddUserResponse* response,
             ::google::protobuf::Closure* done);

void RemoveUser(::google::protobuf::RpcController* controller,
                const ::baidu::galaxy::RemoveUserRequest* request,
                ::baidu::galaxy::RemoveUserResponse* response,
                ::google::protobuf::Closure* done);

void ListUsers(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::ListUsersRequest* request,
               ::baidu::galaxy::ListUsersResponse* response,
               ::google::protobuf::Closure* done);

void ShowUser(::google::protobuf::RpcController* controller,
              const ::baidu::galaxy::ShowUserRequest* request,
              ::baidu::galaxy::ShowUserResponse* response,
              ::google::protobuf::Closure* done);

void GrantUser(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::GrantUserRequest* request,
               ::baidu::galaxy::GrantUserResponse* response,
               ::google::protobuf::Closure* done);

void AssignQuota(::google::protobuf::RpcController* controller,
                 const ::baidu::galaxy::AssignQuotaRequest* request,
                 ::baidu::galaxy::AssignQuotaResponse* response,
                 ::google::protobuf::Closure* done);



};

}
}

#endif

