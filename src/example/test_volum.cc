// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum/volum.h"
#include "protocol/galaxy.pb.h"
#include "util/path_tree.h"

#include <iostream>

int test_dir_volum(const char* source, const char* target);
int test_symlink_volum(const char* source, const char* target);
int test_proc_volum(const char* target);
int test_tmpfs_volum(const char* target, int size, bool readonly);


int main(int argc, char** argv) {

    baidu::galaxy::path::SetRootPath("./");
    
    std::string source;
    std::string target;
    
    std::cerr << "test test_dir_volum ...\n" << std::endl;
    source = "dir_source";
    target = "dir_target";
    //test_dir_volum(source.c_str(), target.c_str());

    //return 0;
    

    std::cerr << "test test_symlink_volum ...\n" << std::endl;
    source = "symlink_source";
    target = "symlink_target";
    test_symlink_volum(source.c_str(), target.c_str());
    
    std::cerr << "test test_tmpfs_volum ...\n" << std::endl;
    target = "target_target";
    //test_tmpfs_volum(target.c_str(), 10240, false);
    return 0;
}

int test_dir_volum(const char* source, const char* target) {
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired());
    vr->set_dest_path(target);
    vr->set_source_path(source);
    vr->set_size(1024);
    vr->set_readonly(false);
    vr->set_medium(baidu::galaxy::proto::kDisk);
    vr->set_type(baidu::galaxy::proto::kEmptyDir);


    baidu::galaxy::volum::Volum volum;
    volum.SetContainerId("container_id");
    volum.SetDescrition(vr);
    

    if (0 != volum.Construct()) {
        std::cerr << "construct volum failed" << std::endl;
        return -1;
    } 

    std::cerr << "construct volum sucessfully" << std::endl;

    if (0 != volum.Mount()) {
        std::cerr << "mount volum failed" << std::endl;
        return -1;
    } 
    std::cerr << "mout volum sucessfully" << std::endl;

    if (0 != volum.Destroy()) {
        std::cerr << "destroy volum failed" << std::endl;
        return -1;
    }
    return 0;
}

int test_symlink_volum(const char* source, const char* target) {
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired());
    vr->set_dest_path(target);
    vr->set_source_path(source);
    vr->set_medium(baidu::galaxy::proto::kDisk);
    vr->set_type(baidu::galaxy::proto::kEmptyDir);
    vr->set_use_symlink(true);

    baidu::galaxy::volum::Volum volum;
    volum.SetContainerId("container_id");
    volum.SetDescrition(vr);
    

    if (0 != volum.Construct()) {
        std::cerr << "construct volum failed" << std::endl;
        return -1;
    } 

    std::cerr << "construct volum sucessfully" << std::endl;

    if (0 != volum.Mount()) {
        std::cerr << "mount volum failed" << std::endl;
        return -1;
    } 
    std::cerr << "mout volum sucessfully" << std::endl;

    if (0 != volum.Destroy()) {
        std::cerr << "destroy volum failed" << std::endl;
        return -1;
    }
    return 0;
}

int test_tmpfs_volum(const char* target, int size, bool readonly) {
     boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr(new baidu::galaxy::proto::VolumRequired());
    vr->set_dest_path(target);
    vr->set_medium(baidu::galaxy::proto::kTmpfs);
    vr->set_type(baidu::galaxy::proto::kEmptyDir);
    vr->set_size(size);
    vr->set_readonly(readonly);
    vr->set_use_symlink(true);

    baidu::galaxy::volum::Volum volum;
    volum.SetContainerId("container_id");
    volum.SetDescrition(vr);
    

    if (0 != volum.Construct()) {
        std::cerr << "construct volum failed" << std::endl;
        return -1;
    } 

    std::cerr << "construct volum sucessfully" << std::endl;

    if (0 != volum.Mount()) {
        std::cerr << "mount volum failed" << std::endl;
        return -1;
    } 
    std::cerr << "mout volum sucessfully" << std::endl;

    if (0 != volum.Destroy()) {
        std::cerr << "destroy volum failed" << std::endl;
        return -1;
    }
    return 0;
}

