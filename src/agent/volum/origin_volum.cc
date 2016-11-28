#include "origin_volum.h"

#include "protocol/galaxy.pb.h"
#include "mounter.h"
#include "util/user.h"
#include "util/util.h"
#include "glog/logging.h"
#include "collector/collector_engine.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <sys/mount.h>

namespace baidu {
namespace galaxy {
namespace volum {

OriginVolum::OriginVolum() {}

OriginVolum::~OriginVolum() {}

baidu::galaxy::util::ErrorCode OriginVolum::Construct() {
    VLOG(10) << "origin volum construct";
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    assert(vr->medium() == baidu::galaxy::proto::kDisk
            || vr->medium() == baidu::galaxy::proto::kSsd);
    boost::system::error_code ec;
    boost::filesystem::path target_path(this->TargetPath());

    if (!boost::filesystem::exists(target_path, ec)
            && !baidu::galaxy::file::create_directories(target_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                target_path.string().c_str(),
                ec.message().c_str());
    }

    boost::filesystem::path source_path(vr->source_path());
    if (!boost::filesystem::exists(source_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                source_path.string().c_str(),
                ec.message().c_str());
    }

    std::map<std::string, boost::shared_ptr<Mounter> > mounters;
    ListMounters(mounters);
    std::map<std::string, boost::shared_ptr<Mounter> >::iterator iter = mounters.find(target_path.string());

    if (iter != mounters.end()) {
        return ERRORCODE(0, "already bind");
    }

    return MountDir(source_path.string(), target_path.string());
}


baidu::galaxy::util::ErrorCode OriginVolum::Destroy() {
    if (NULL != vc_.get()) {
        vc_->Enable(false);
    }

    baidu::galaxy::util::ErrorCode err = Umount(this->TargetPath());

    if (err.Code() != 0) {
        return err;
    }

    return ERRORCODE_OK;
}

int64_t OriginVolum::Used() {
    return 0L;
}

baidu::galaxy::util::ErrorCode OriginVolum::Gc() {
    return ERRORCODE_OK;
}

std::string OriginVolum::ToString() {
    return "";
}

}
}
}

