#include "resman_impl.h"
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

ResManImpl::ResManImpl() {

}

ResManImpl::~ResManImpl() {

}

void ResManImpl::EnterSafeMode(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::EnterSafeModeRequest* request,
                               ::baidu::galaxy::EnterSafeModeResponse* response,
                               ::google::protobuf::Closure* done) {

}


void ResManImpl::LeaveSafeMode(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::LeaveSafeModeRequest* request,
                               ::baidu::galaxy::LeaveSafeModeResponse* response,
                               ::google::protobuf::Closure* done) {

}


void ResManImpl::Status(::google::protobuf::RpcController* controller,
                        const ::baidu::galaxy::StatusRequest* request,
                        ::baidu::galaxy::StatusResponse* response,
                        ::google::protobuf::Closure* done) {

}


void ResManImpl::KeepAlive(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::KeepAliveRequest* request,
                           ::baidu::galaxy::KeepAliveResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::SubmitJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::SubmitJobRequest* request,
                           ::baidu::galaxy::SubmitJobResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::UpdateJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::UpdateJobRequest* request,
                           ::baidu::galaxy::UpdateJobResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::RemoveJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::RemoveJobRequest* request,
                           ::baidu::galaxy::RemoveJobResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::AddAgent(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::AddAgentRequest* request,
                          ::baidu::galaxy::AddAgentResponse* response,
                          ::google::protobuf::Closure* done) {

}


void ResManImpl::RemoveAgent(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::RemoveAgentRequest* request,
                             ::baidu::galaxy::RemoveAgentResponse* response,
                             ::google::protobuf::Closure* done) {

}


void ResManImpl::OnlineAgent(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::OnlineAgentRequest* request,
                             ::baidu::galaxy::OnlineAgentResponse* response,
                             ::google::protobuf::Closure* done) {

}


void ResManImpl::OfflineAgent(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::OfflineAgentRequest* request,
                              ::baidu::galaxy::OfflineAgentResponse* response,
                              ::google::protobuf::Closure* done) {

}


void ResManImpl::ListAgents(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::ListAgentsRequest* request,
                            ::baidu::galaxy::ListAgentsResponse* response,
                            ::google::protobuf::Closure* done) {

}


void ResManImpl::CreateTag(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::CreateTagRequest* request,
                           ::baidu::galaxy::CreateTagResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::RemoveTag(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::RemoveTagRequest* request,
                           ::baidu::galaxy::RemoveTagResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::ListTags(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::ListTagsRequest* request,
                          ::baidu::galaxy::ListTagsResponse* response,
                          ::google::protobuf::Closure* done) {

}


void ResManImpl::AddAgentToTag(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::AddAgentToTagRequest* request,
                               ::baidu::galaxy::AddAgentToTagResponse* response,
                               ::google::protobuf::Closure* done) {

}


void ResManImpl::RemoveAgentFromTag(::google::protobuf::RpcController* controller,
                                    const ::baidu::galaxy::RemoveAgentFromTagRequest* request,
                                    ::baidu::galaxy::RemoveAgentFromTagResponse* response,
                                    ::google::protobuf::Closure* done) {

}


void ResManImpl::ListAgentsByTag(::google::protobuf::RpcController* controller,
                                 const ::baidu::galaxy::ListAgentsByTagRequest* request,
                                 ::baidu::galaxy::ListAgentsByTagResponse* response,
                                 ::google::protobuf::Closure* done) {

}


void ResManImpl::AddUser(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::AddUserRequest* request,
                         ::baidu::galaxy::AddUserResponse* response,
                         ::google::protobuf::Closure* done) {

}


void ResManImpl::RemoveUser(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::RemoveUserRequest* request,
                            ::baidu::galaxy::RemoveUserResponse* response,
                            ::google::protobuf::Closure* done) {

}


void ResManImpl::ListUsers(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::ListUsersRequest* request,
                           ::baidu::galaxy::ListUsersResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::ShowUser(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::ShowUserRequest* request,
                          ::baidu::galaxy::ShowUserResponse* response,
                          ::google::protobuf::Closure* done) {

}


void ResManImpl::GrantUser(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::GrantUserRequest* request,
                           ::baidu::galaxy::GrantUserResponse* response,
                           ::google::protobuf::Closure* done) {

}


void ResManImpl::AssignQuota(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::AssignQuotaRequest* request,
                             ::baidu::galaxy::AssignQuotaResponse* response,
                             ::google::protobuf::Closure* done) {

}


}
}


