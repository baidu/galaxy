// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scheduler/scheduler_io.h"

#include "logging.h"

namespace baidu {
namespace galaxy {

void SchedulerIO::Loop() {
	GetResourceSnapshotRequest sync_request;
	GetResourceSnapshotResponse sync_response;
	bool ret = rpc_client_.SendRequest(master_stub_,
									  &Master_Stub::GetResourceSnapshot,
									  &sync_request, &sync_response, 5, 1);
	if (!ret) {
		LOG(WARNING, "fail to get master resource snapshot");
		return;
	}
	int32_t status = scheduler_.SyncResources(&sync_response);
	if (status != 0) {
		LOG(WARNING, "fail to sync resource from master");
		return;
	}
	LOG(INFO, "sync resource from master  successfully");

	GetPendingJobsRequest get_jobs_request;
	GetPendingJobsResponse get_jobs_response;

	ProposeRequest pro_request;
	ProposeResponse pro_response;
	ret = rpc_client_.SendRequest(master_stub_,
			 	 	 	 	 	 &Master_Stub::GetPendingJobs,
								 &get_jobs_request,
								 &get_jobs_response,
								 5, 1);
	if (!ret) {
		LOG(WARNING, "fail to get pending jobs from master");
	}
	LOG(INFO, "get pending jobs from master , pending_size %d, scale_down_size %d",
			get_jobs_response.scale_up_jobs_size(),
			get_jobs_response.scale_down_jobs_size());
	std::vector<JobInfo*> pending_jobs;
	for (int i = 0; i < get_jobs_response.scale_up_jobs_size(); i++) {
		pending_jobs.push_back(get_jobs_response.mutable_scale_up_jobs(i));
	}
	std::vector<JobInfo*> reducing_jobs;
	for (int i = 0; i < get_jobs_response.scale_down_jobs_size(); i++) {
		reducing_jobs.push_back(get_jobs_response.mutable_scale_down_jobs(i));
	}
	std::vector<ScheduleInfo*> propose;
	status = scheduler_.ScheduleScaleUp(pending_jobs, &propose);
	if (status != 0) {
		LOG(INFO, "fail to schedule scale up ");
		goto END;
	}
	status = scheduler_.ScheduleScaleDown(reducing_jobs, &propose);
	if (status != 0) {
		LOG(INFO, "fail to schedule scale down ");
		goto END;
	}

	for (std::vector<ScheduleInfo*>::iterator it = propose.begin();
			it != propose.end(); ++it) {
		ScheduleInfo* sched = pro_request.add_schedule();
		sched->CopyFrom(*(*it));
	}
	ret = rpc_client_.SendRequest(master_stub_,
				 	 	 	 	  &Master_Stub::Propose,
								  &pro_request,
								  &pro_response,
								  5, 1);
	if (!ret) {
		LOG(INFO, "fail to propose");
	}
END:
	for (std::vector<ScheduleInfo*>::iterator it = propose.begin();
				it != propose.end(); ++it) {
		delete *it;
    }
	propose.clear();
}

} // galaxy
} // baidu
