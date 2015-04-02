#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS \
  -DVERSION=\"1.9.8.7\"
INCPATH=-I. \
  -I./src \
  -I./output/include
DEP_INCPATH=-I../../../public/sofa-pbrpc \
  -I../../../public/sofa-pbrpc/include \
  -I../../../public/sofa-pbrpc/output \
  -I../../../public/sofa-pbrpc/output/include \
  -I../../../third-64/libcurl \
  -I../../../third-64/libcurl/include \
  -I../../../third-64/libcurl/output \
  -I../../../third-64/libcurl/output/include \
  -I../../../third-64/protobuf \
  -I../../../third-64/protobuf/include \
  -I../../../third-64/protobuf/output \
  -I../../../third-64/protobuf/output/include \
  -I../../../third-64/snappy \
  -I../../../third-64/snappy/include \
  -I../../../third-64/snappy/output \
  -I../../../third-64/snappy/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=74fdd0194a2842c615b3761ef75a8cd6  COMAKE


.PHONY:all
all:comake2_makefile_check agent master libgalaxy.a galaxy_client curl_downloader_test downloader_mgr_test 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:comake2_makefile_check
comake2_makefile_check:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcomake2_makefile_check[0m']"
	#in case of error, update 'Makefile' by 'comake2'
	@echo "$(COMAKE_MD5)">comake2.md5
	@md5sum -c --status comake2.md5
	@rm -f comake2.md5

.PHONY:ccpclean
ccpclean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mccpclean[0m']"
	@echo "make ccpclean done"

.PHONY:clean
clean:ccpclean
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf agent
	rm -rf ./output/bin/agent
	rm -rf master
	rm -rf ./output/bin/master
	rm -rf libgalaxy.a
	rm -rf ./output/lib/libgalaxy.a
	rm -rf ./output/include/galaxy.h
	rm -rf galaxy_client
	rm -rf ./output/bin/galaxy_client
	rm -rf curl_downloader_test
	rm -rf ./output/bin/curl_downloader_test
	rm -rf downloader_mgr_test
	rm -rf ./output/bin/downloader_mgr_test
	rm -rf src/agent_flags.o
	rm -rf common/agent_logging.o
	rm -rf common/agent_util.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/agent_task.pb.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/agent_agent.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/agent_master.pb.o
	rm -rf src/agent/agent_task_runner.o
	rm -rf src/agent/agent_task_manager.o
	rm -rf src/agent/agent_cgroup.o
	rm -rf src/agent/agent_workspace.o
	rm -rf src/agent/agent_workspace_manager.o
	rm -rf src/agent/agent_agent_impl.o
	rm -rf src/agent/agent_agent_main.o
	rm -rf src/agent/agent_downloader_manager.o
	rm -rf src/agent/agent_curl_downloader.o
	rm -rf src/agent/agent_binary_downloader.o
	rm -rf src/master_flags.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/master_task.pb.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/master_agent.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/master_master.pb.o
	rm -rf common/master_logging.o
	rm -rf src/master/master_master_impl.o
	rm -rf src/master/master_master_main.o
	rm -rf common/galaxy_logging.o
	rm -rf src/sdk/galaxy_galaxy.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/galaxy_task.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/galaxy_master.pb.o
	rm -rf src/client/galaxy_client_galaxy_client.o
	rm -rf src/agent/curl_downloader_test_curl_downloader.o
	rm -rf test/curl_downloader_test_curl_downloader_test.o
	rm -rf common/curl_downloader_test_logging.o
	rm -rf src/agent/downloader_mgr_test_curl_downloader.o
	rm -rf src/agent/downloader_mgr_test_downloader_manager.o
	rm -rf test/downloader_mgr_test_downloader_manager_test.o
	rm -rf common/downloader_mgr_test_logging.o
	rm -rf src/agent/downloader_mgr_test_binary_downloader.o

.PHONY:dist
dist:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdist[0m']"
	tar czvf output.tar.gz output
	@echo "make dist done"

.PHONY:distclean
distclean:clean
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdistclean[0m']"
	rm -f output.tar.gz
	@echo "make distclean done"

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

agent:src/agent_flags.o \
  common/agent_logging.o \
  common/agent_util.o \
  src/proto/agent_task.pb.o \
  src/proto/agent_agent.pb.o \
  src/proto/agent_master.pb.o \
  src/agent/agent_task_runner.o \
  src/agent/agent_task_manager.o \
  src/agent/agent_cgroup.o \
  src/agent/agent_workspace.o \
  src/agent/agent_workspace_manager.o \
  src/agent/agent_agent_impl.o \
  src/agent/agent_agent_main.o \
  src/agent/agent_downloader_manager.o \
  src/agent/agent_curl_downloader.o \
  src/agent/agent_binary_downloader.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent[0m']"
	$(CXX) src/agent_flags.o \
  common/agent_logging.o \
  common/agent_util.o \
  src/proto/agent_task.pb.o \
  src/proto/agent_agent.pb.o \
  src/proto/agent_master.pb.o \
  src/agent/agent_task_runner.o \
  src/agent/agent_task_manager.o \
  src/agent/agent_cgroup.o \
  src/agent/agent_workspace.o \
  src/agent/agent_workspace_manager.o \
  src/agent/agent_agent_impl.o \
  src/agent/agent_agent_main.o \
  src/agent/agent_downloader_manager.o \
  src/agent/agent_curl_downloader.o \
  src/agent/agent_binary_downloader.o -Xlinker "-("  ../../../public/sofa-pbrpc/lib/libsofa_pbrpc.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap -Xlinker "-)" -o agent
	mkdir -p ./output/bin
	cp -f --link agent ./output/bin

master:src/master_flags.o \
  src/proto/master_task.pb.o \
  src/proto/master_agent.pb.o \
  src/proto/master_master.pb.o \
  common/master_logging.o \
  src/master/master_master_impl.o \
  src/master/master_master_main.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mmaster[0m']"
	$(CXX) src/master_flags.o \
  src/proto/master_task.pb.o \
  src/proto/master_agent.pb.o \
  src/proto/master_master.pb.o \
  common/master_logging.o \
  src/master/master_master_impl.o \
  src/master/master_master_main.o -Xlinker "-("  ../../../public/sofa-pbrpc/lib/libsofa_pbrpc.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap -Xlinker "-)" -o master
	mkdir -p ./output/bin
	cp -f --link master ./output/bin

libgalaxy.a:common/galaxy_logging.o \
  src/sdk/galaxy_galaxy.o \
  src/proto/galaxy_task.pb.o \
  src/proto/galaxy_master.pb.o \
  src/sdk/galaxy.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibgalaxy.a[0m']"
	ar crs libgalaxy.a common/galaxy_logging.o \
  src/sdk/galaxy_galaxy.o \
  src/proto/galaxy_task.pb.o \
  src/proto/galaxy_master.pb.o
	mkdir -p ./output/lib
	cp -f --link libgalaxy.a ./output/lib
	mkdir -p ./output/include
	cp -f --link src/sdk/galaxy.h ./output/include

galaxy_client:src/client/galaxy_client_galaxy_client.o \
  ./libgalaxy.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mgalaxy_client[0m']"
	$(CXX) src/client/galaxy_client_galaxy_client.o -Xlinker "-(" ./libgalaxy.a ../../../public/sofa-pbrpc/lib/libsofa_pbrpc.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap -Xlinker "-)" -o galaxy_client
	mkdir -p ./output/bin
	cp -f --link galaxy_client ./output/bin

curl_downloader_test:src/agent/curl_downloader_test_curl_downloader.o \
  test/curl_downloader_test_curl_downloader_test.o \
  common/curl_downloader_test_logging.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcurl_downloader_test[0m']"
	$(CXX) src/agent/curl_downloader_test_curl_downloader.o \
  test/curl_downloader_test_curl_downloader_test.o \
  common/curl_downloader_test_logging.o -Xlinker "-("  ../../../public/sofa-pbrpc/lib/libsofa_pbrpc.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap -Xlinker "-)" -o curl_downloader_test
	mkdir -p ./output/bin
	cp -f --link curl_downloader_test ./output/bin

downloader_mgr_test:src/agent/downloader_mgr_test_curl_downloader.o \
  src/agent/downloader_mgr_test_downloader_manager.o \
  test/downloader_mgr_test_downloader_manager_test.o \
  common/downloader_mgr_test_logging.o \
  src/agent/downloader_mgr_test_binary_downloader.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdownloader_mgr_test[0m']"
	$(CXX) src/agent/downloader_mgr_test_curl_downloader.o \
  src/agent/downloader_mgr_test_downloader_manager.o \
  test/downloader_mgr_test_downloader_manager_test.o \
  common/downloader_mgr_test_logging.o \
  src/agent/downloader_mgr_test_binary_downloader.o -Xlinker "-("  ../../../public/sofa-pbrpc/lib/libsofa_pbrpc.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap -Xlinker "-)" -o downloader_mgr_test
	mkdir -p ./output/bin
	cp -f --link downloader_mgr_test ./output/bin

src/agent_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent_flags.o src/flags.cc

common/agent_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_logging.o common/logging.cc

common/agent_util.o:common/util.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_util.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_util.o common/util.cc

src/proto/task.pb.cc \
  src/proto/task.pb.h:src/proto/task.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/task.pb.cc \
  src/proto/task.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/task.proto

src/proto/task.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/task.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/agent_task.pb.o:src/proto/task.pb.cc \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_task.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_task.pb.o src/proto/task.pb.cc

src/proto/agent.pb.cc \
  src/proto/agent.pb.h:src/proto/agent.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent.pb.cc \
  src/proto/agent.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/agent.proto

src/proto/agent.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/agent_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_agent.pb.o src/proto/agent.pb.cc

src/proto/master.pb.cc \
  src/proto/master.pb.h:src/proto/master.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master.pb.cc \
  src/proto/master.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/master.proto

src/proto/master.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/agent_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_master.pb.o src/proto/master.pb.cc

src/agent/agent_task_runner.o:src/agent/task_runner.cc \
  src/agent/task_runner.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/workspace.h \
  common/logging.h \
  common/util.h \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_task_runner.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_task_runner.o src/agent/task_runner.cc

src/agent/agent_task_manager.o:src/agent/task_manager.cc \
  src/agent/task_manager.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/task_runner.h \
  src/agent/workspace.h \
  common/logging.h \
  src/agent/cgroup.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_task_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_task_manager.o src/agent/task_manager.cc

src/agent/agent_cgroup.o:src/agent/cgroup.cc \
  src/agent/cgroup.h \
  src/agent/task_runner.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/workspace.h \
  common/logging.h \
  common/util.h \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_cgroup.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_cgroup.o src/agent/cgroup.cc

src/agent/agent_workspace.o:src/agent/workspace.cc \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_workspace.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_workspace.o src/agent/workspace.cc

src/agent/agent_workspace_manager.o:src/agent/workspace_manager.cc \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_workspace_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_workspace_manager.o src/agent/workspace_manager.cc

src/agent/agent_agent_impl.o:src/agent/agent_impl.cc \
  src/agent/agent_impl.h \
  src/proto/agent.pb.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/task_manager.h \
  src/agent/task_runner.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/workspace.h \
  common/util.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/thread_pool.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_agent_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_agent_impl.o src/agent/agent_impl.cc

src/agent/agent_agent_main.o:src/agent/agent_main.cc \
  src/agent/agent_impl.h \
  src/proto/agent.pb.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/task_manager.h \
  src/agent/task_runner.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/workspace.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_agent_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_agent_main.o src/agent/agent_main.cc

src/agent/agent_downloader_manager.o:src/agent/downloader_manager.cc \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/asm_atomic.h \
  src/agent/curl_downloader.h \
  src/agent/binary_downloader.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_downloader_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_downloader_manager.o src/agent/downloader_manager.cc

src/agent/agent_curl_downloader.o:src/agent/curl_downloader.cc \
  src/agent/curl_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_curl_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_curl_downloader.o src/agent/curl_downloader.cc

src/agent/agent_binary_downloader.o:src/agent/binary_downloader.cc \
  src/agent/binary_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_binary_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_binary_downloader.o src/agent/binary_downloader.cc

src/master_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master_flags.o src/flags.cc

src/proto/master_task.pb.o:src/proto/task.pb.cc \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_task.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_task.pb.o src/proto/task.pb.cc

src/proto/master_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_agent.pb.o src/proto/agent.pb.cc

src/proto/master_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_master.pb.o src/proto/master.pb.cc

common/master_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/master_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/master_logging.o common/logging.cc

src/master/master_master_impl.o:src/master/master_impl.cc \
  src/master/master_impl.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  src/proto/agent.pb.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/thread_pool.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_impl.o src/master/master_impl.cc

src/master/master_master_main.o:src/master/master_main.cc \
  src/master/master_impl.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_main.o src/master/master_main.cc

common/galaxy_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/galaxy_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/galaxy_logging.o common/logging.cc

src/sdk/galaxy_galaxy.o:src/sdk/galaxy.cc \
  src/sdk/galaxy.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/sdk/galaxy_galaxy.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/sdk/galaxy_galaxy.o src/sdk/galaxy.cc

src/proto/galaxy_task.pb.o:src/proto/task.pb.cc \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/galaxy_task.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/galaxy_task.pb.o src/proto/task.pb.cc

src/proto/galaxy_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/galaxy_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/galaxy_master.pb.o src/proto/master.pb.cc

src/client/galaxy_client_galaxy_client.o:src/client/galaxy_client.cc \
  src/sdk/galaxy.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/client/galaxy_client_galaxy_client.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/client/galaxy_client_galaxy_client.o src/client/galaxy_client.cc

src/agent/curl_downloader_test_curl_downloader.o:src/agent/curl_downloader.cc \
  src/agent/curl_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/curl_downloader_test_curl_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/curl_downloader_test_curl_downloader.o src/agent/curl_downloader.cc

test/curl_downloader_test_curl_downloader_test.o:test/curl_downloader_test.cc \
  src/agent/curl_downloader.h \
  src/agent/downloader.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/curl_downloader_test_curl_downloader_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/curl_downloader_test_curl_downloader_test.o test/curl_downloader_test.cc

common/curl_downloader_test_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/curl_downloader_test_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/curl_downloader_test_logging.o common/logging.cc

src/agent/downloader_mgr_test_curl_downloader.o:src/agent/curl_downloader.cc \
  src/agent/curl_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/downloader_mgr_test_curl_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/downloader_mgr_test_curl_downloader.o src/agent/curl_downloader.cc

src/agent/downloader_mgr_test_downloader_manager.o:src/agent/downloader_manager.cc \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/asm_atomic.h \
  src/agent/curl_downloader.h \
  src/agent/binary_downloader.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/downloader_mgr_test_downloader_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/downloader_mgr_test_downloader_manager.o src/agent/downloader_manager.cc

test/downloader_mgr_test_downloader_manager_test.o:test/downloader_manager_test.cc \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/downloader_mgr_test_downloader_manager_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/downloader_mgr_test_downloader_manager_test.o test/downloader_manager_test.cc

common/downloader_mgr_test_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/downloader_mgr_test_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/downloader_mgr_test_logging.o common/logging.cc

src/agent/downloader_mgr_test_binary_downloader.o:src/agent/binary_downloader.cc \
  src/agent/binary_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/downloader_mgr_test_binary_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/downloader_mgr_test_binary_downloader.o src/agent/binary_downloader.cc

endif #ifeq ($(shell uname -m),x86_64)


