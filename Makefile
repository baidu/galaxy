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
  -Werror \
  -fPIC
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS \
  -DVERSION=\"1.9.8.7\" \
  -D_BD_KERNEL_
INCPATH=-I. \
  -I./src \
  -I./output/include
DEP_INCPATH=-I../../../public/sofa-pbrpc \
  -I../../../public/sofa-pbrpc/include \
  -I../../../public/sofa-pbrpc/output \
  -I../../../public/sofa-pbrpc/output/include \
  -I../../../third-64/boost \
  -I../../../third-64/boost/include \
  -I../../../third-64/boost/output \
  -I../../../third-64/boost/output/include \
  -I../../../third-64/gflags \
  -I../../../third-64/gflags/include \
  -I../../../third-64/gflags/output \
  -I../../../third-64/gflags/output/include \
  -I../../../third-64/gtest \
  -I../../../third-64/gtest/include \
  -I../../../third-64/gtest/output \
  -I../../../third-64/gtest/output/include \
  -I../../../third-64/leveldb \
  -I../../../third-64/leveldb/include \
  -I../../../third-64/leveldb/output \
  -I../../../third-64/leveldb/output/include \
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
COMAKE_MD5=f5a05aecd6e517c8b9bd7a347bb0e417  COMAKE


.PHONY:all
all:comake2_makefile_check agent master libgalaxy.a galaxy_client curl_downloader_test downloader_mgr_test resource_collector_test agent_utils_test agent_utils_remove_test agent_mock master_mock monitor_agent agent_password_test gssh_password libpam_galaxy.so 
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
	rm -rf resource_collector_test
	rm -rf ./output/bin/resource_collector_test
	rm -rf agent_utils_test
	rm -rf ./output/bin/agent_utils_test
	rm -rf agent_utils_remove_test
	rm -rf ./output/bin/agent_utils_remove_test
	rm -rf agent_mock
	rm -rf ./output/bin/agent_mock
	rm -rf master_mock
	rm -rf ./output/bin/master_mock
	rm -rf monitor_agent
	rm -rf ./output/bin/monitor_agent
	rm -rf agent_password_test
	rm -rf ./output/bin/agent_password_test
	rm -rf gssh_password
	rm -rf ./output/bin/gssh_password
	rm -rf libpam_galaxy.so
	rm -rf ./output/so/libpam_galaxy.so
	rm -rf src/agent_flags.o
	rm -rf common/agent_logging.o
	rm -rf common/agent_util.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/agent_task.pb.o
	rm -rf src/proto/monitor.pb.cc
	rm -rf src/proto/monitor.pb.h
	rm -rf src/proto/agent_monitor.pb.o
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
	rm -rf src/agent/agent_resource_collector.o
	rm -rf src/agent/agent_resource_manager.o
	rm -rf src/agent/agent_resource_collector_engine.o
	rm -rf src/agent/agent_utils.o
	rm -rf src/agent/agent_dynamic_resource_scheduler.o
	rm -rf src/master_flags.o
	rm -rf src/proto/monitor.pb.cc
	rm -rf src/proto/monitor.pb.h
	rm -rf src/proto/master_monitor.pb.o
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
	rm -rf src/curl_downloader_test_flags.o
	rm -rf src/agent/downloader_mgr_test_curl_downloader.o
	rm -rf src/agent/downloader_mgr_test_downloader_manager.o
	rm -rf src/downloader_mgr_test_flags.o
	rm -rf test/downloader_mgr_test_downloader_manager_test.o
	rm -rf common/downloader_mgr_test_logging.o
	rm -rf src/agent/downloader_mgr_test_binary_downloader.o
	rm -rf src/agent/resource_collector_test_resource_collector.o
	rm -rf test/resource_collector_test_resource_collector_test.o
	rm -rf common/resource_collector_test_logging.o
	rm -rf src/agent/resource_collector_test_resource_collector_engine.o
	rm -rf src/resource_collector_test_flags.o
	rm -rf src/agent/agent_utils_test_utils.o
	rm -rf common/agent_utils_test_logging.o
	rm -rf test/agent_utils_test_agent_utils_test.o
	rm -rf src/agent/agent_utils_remove_test_utils.o
	rm -rf common/agent_utils_remove_test_logging.o
	rm -rf test/agent_utils_remove_test_agent_utils_remove_test.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/agent_mock_master.pb.o
	rm -rf common/agent_mock_logging.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/agent_mock_task.pb.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/agent_mock_agent.pb.o
	rm -rf test/agent_mock_pb_file_parser.o
	rm -rf test/agent_mock_agent_test_mock.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/master_mock_master.pb.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/master_mock_task.pb.o
	rm -rf test/master_mock_pb_file_parser.o
	rm -rf test/master_mock_master_test_mock.o
	rm -rf common/monitor_agent_logging.o
	rm -rf common/monitor_agent_util.o
	rm -rf src/monitor_agent_flags.o
	rm -rf src/monitor/monitor_agent_monitor_main.o
	rm -rf src/monitor/monitor_agent_monitor_impl.o
	rm -rf src/proto/monitor.pb.cc
	rm -rf src/proto/monitor.pb.h
	rm -rf src/proto/monitor_agent_monitor.pb.o
	rm -rf src/agent_password_test_flags.o
	rm -rf common/agent_password_test_logging.o
	rm -rf common/agent_password_test_util.o
	rm -rf src/proto/task.pb.cc
	rm -rf src/proto/task.pb.h
	rm -rf src/proto/agent_password_test_task.pb.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/agent_password_test_agent.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/agent_password_test_master.pb.o
	rm -rf src/agent/agent_password_test_task_runner.o
	rm -rf src/agent/agent_password_test_task_manager.o
	rm -rf src/agent/agent_password_test_cgroup.o
	rm -rf src/agent/agent_password_test_workspace.o
	rm -rf src/agent/agent_password_test_workspace_manager.o
	rm -rf src/agent/agent_password_test_agent_impl.o
	rm -rf src/agent/agent_password_test_downloader_manager.o
	rm -rf src/agent/agent_password_test_curl_downloader.o
	rm -rf src/agent/agent_password_test_binary_downloader.o
	rm -rf src/agent/agent_password_test_resource_collector.o
	rm -rf src/agent/agent_password_test_resource_manager.o
	rm -rf src/agent/agent_password_test_resource_collector_engine.o
	rm -rf src/agent/agent_password_test_utils.o
	rm -rf src/agent/agent_password_test_dynamic_resource_scheduler.o
	rm -rf src/gssh/agent_password_test_agent_password_test.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/gssh_password_agent.pb.o
	rm -rf common/gssh_password_logging.o
	rm -rf src/gssh/gssh_password_gssh_password.o
	rm -rf src/gssh/pam_galaxy_pam_galaxy.o

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
  src/proto/agent_monitor.pb.o \
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
  src/agent/agent_binary_downloader.o \
  src/agent/agent_resource_collector.o \
  src/agent/agent_resource_manager.o \
  src/agent/agent_resource_collector_engine.o \
  src/agent/agent_utils.o \
  src/agent/agent_dynamic_resource_scheduler.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent[0m']"
	$(CXX) src/agent_flags.o \
  common/agent_logging.o \
  common/agent_util.o \
  src/proto/agent_task.pb.o \
  src/proto/agent_monitor.pb.o \
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
  src/agent/agent_binary_downloader.o \
  src/agent/agent_resource_collector.o \
  src/agent/agent_resource_manager.o \
  src/agent/agent_resource_collector_engine.o \
  src/agent/agent_utils.o \
  src/agent/agent_dynamic_resource_scheduler.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o agent
	mkdir -p ./output/bin
	cp -f --link agent ./output/bin

master:src/master_flags.o \
  src/proto/master_monitor.pb.o \
  src/proto/master_task.pb.o \
  src/proto/master_agent.pb.o \
  src/proto/master_master.pb.o \
  common/master_logging.o \
  src/master/master_master_impl.o \
  src/master/master_master_main.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mmaster[0m']"
	$(CXX) src/master_flags.o \
  src/proto/master_monitor.pb.o \
  src/proto/master_task.pb.o \
  src/proto/master_agent.pb.o \
  src/proto/master_master.pb.o \
  common/master_logging.o \
  src/master/master_master_impl.o \
  src/master/master_master_main.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o master
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
	$(CXX) src/client/galaxy_client_galaxy_client.o -Xlinker "-(" ./libgalaxy.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o galaxy_client
	mkdir -p ./output/bin
	cp -f --link galaxy_client ./output/bin

curl_downloader_test:src/agent/curl_downloader_test_curl_downloader.o \
  test/curl_downloader_test_curl_downloader_test.o \
  common/curl_downloader_test_logging.o \
  src/curl_downloader_test_flags.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcurl_downloader_test[0m']"
	$(CXX) src/agent/curl_downloader_test_curl_downloader.o \
  test/curl_downloader_test_curl_downloader_test.o \
  common/curl_downloader_test_logging.o \
  src/curl_downloader_test_flags.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o curl_downloader_test
	mkdir -p ./output/bin
	cp -f --link curl_downloader_test ./output/bin

downloader_mgr_test:src/agent/downloader_mgr_test_curl_downloader.o \
  src/agent/downloader_mgr_test_downloader_manager.o \
  src/downloader_mgr_test_flags.o \
  test/downloader_mgr_test_downloader_manager_test.o \
  common/downloader_mgr_test_logging.o \
  src/agent/downloader_mgr_test_binary_downloader.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdownloader_mgr_test[0m']"
	$(CXX) src/agent/downloader_mgr_test_curl_downloader.o \
  src/agent/downloader_mgr_test_downloader_manager.o \
  src/downloader_mgr_test_flags.o \
  test/downloader_mgr_test_downloader_manager_test.o \
  common/downloader_mgr_test_logging.o \
  src/agent/downloader_mgr_test_binary_downloader.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o downloader_mgr_test
	mkdir -p ./output/bin
	cp -f --link downloader_mgr_test ./output/bin

resource_collector_test:src/agent/resource_collector_test_resource_collector.o \
  test/resource_collector_test_resource_collector_test.o \
  common/resource_collector_test_logging.o \
  src/agent/resource_collector_test_resource_collector_engine.o \
  src/resource_collector_test_flags.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mresource_collector_test[0m']"
	$(CXX) src/agent/resource_collector_test_resource_collector.o \
  test/resource_collector_test_resource_collector_test.o \
  common/resource_collector_test_logging.o \
  src/agent/resource_collector_test_resource_collector_engine.o \
  src/resource_collector_test_flags.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o resource_collector_test
	mkdir -p ./output/bin
	cp -f --link resource_collector_test ./output/bin

agent_utils_test:src/agent/agent_utils_test_utils.o \
  common/agent_utils_test_logging.o \
  test/agent_utils_test_agent_utils_test.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent_utils_test[0m']"
	$(CXX) src/agent/agent_utils_test_utils.o \
  common/agent_utils_test_logging.o \
  test/agent_utils_test_agent_utils_test.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o agent_utils_test
	mkdir -p ./output/bin
	cp -f --link agent_utils_test ./output/bin

agent_utils_remove_test:src/agent/agent_utils_remove_test_utils.o \
  common/agent_utils_remove_test_logging.o \
  test/agent_utils_remove_test_agent_utils_remove_test.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent_utils_remove_test[0m']"
	$(CXX) src/agent/agent_utils_remove_test_utils.o \
  common/agent_utils_remove_test_logging.o \
  test/agent_utils_remove_test_agent_utils_remove_test.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o agent_utils_remove_test
	mkdir -p ./output/bin
	cp -f --link agent_utils_remove_test ./output/bin

agent_mock:src/proto/agent_mock_master.pb.o \
  common/agent_mock_logging.o \
  src/proto/agent_mock_task.pb.o \
  src/proto/agent_mock_agent.pb.o \
  test/agent_mock_pb_file_parser.o \
  test/agent_mock_agent_test_mock.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent_mock[0m']"
	$(CXX) src/proto/agent_mock_master.pb.o \
  common/agent_mock_logging.o \
  src/proto/agent_mock_task.pb.o \
  src/proto/agent_mock_agent.pb.o \
  test/agent_mock_pb_file_parser.o \
  test/agent_mock_agent_test_mock.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o agent_mock
	mkdir -p ./output/bin
	cp -f --link agent_mock ./output/bin

master_mock:src/proto/master_mock_master.pb.o \
  src/proto/master_mock_task.pb.o \
  test/master_mock_pb_file_parser.o \
  test/master_mock_master_test_mock.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mmaster_mock[0m']"
	$(CXX) src/proto/master_mock_master.pb.o \
  src/proto/master_mock_task.pb.o \
  test/master_mock_pb_file_parser.o \
  test/master_mock_master_test_mock.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o master_mock
	mkdir -p ./output/bin
	cp -f --link master_mock ./output/bin

monitor_agent:common/monitor_agent_logging.o \
  common/monitor_agent_util.o \
  src/monitor_agent_flags.o \
  src/monitor/monitor_agent_monitor_main.o \
  src/monitor/monitor_agent_monitor_impl.o \
  src/proto/monitor_agent_monitor.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mmonitor_agent[0m']"
	$(CXX) common/monitor_agent_logging.o \
  common/monitor_agent_util.o \
  src/monitor_agent_flags.o \
  src/monitor/monitor_agent_monitor_main.o \
  src/monitor/monitor_agent_monitor_impl.o \
  src/proto/monitor_agent_monitor.pb.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o monitor_agent
	mkdir -p ./output/bin
	cp -f --link monitor_agent ./output/bin

agent_password_test:src/agent_password_test_flags.o \
  common/agent_password_test_logging.o \
  common/agent_password_test_util.o \
  src/proto/agent_password_test_task.pb.o \
  src/proto/agent_password_test_agent.pb.o \
  src/proto/agent_password_test_master.pb.o \
  src/agent/agent_password_test_task_runner.o \
  src/agent/agent_password_test_task_manager.o \
  src/agent/agent_password_test_cgroup.o \
  src/agent/agent_password_test_workspace.o \
  src/agent/agent_password_test_workspace_manager.o \
  src/agent/agent_password_test_agent_impl.o \
  src/agent/agent_password_test_downloader_manager.o \
  src/agent/agent_password_test_curl_downloader.o \
  src/agent/agent_password_test_binary_downloader.o \
  src/agent/agent_password_test_resource_collector.o \
  src/agent/agent_password_test_resource_manager.o \
  src/agent/agent_password_test_resource_collector_engine.o \
  src/agent/agent_password_test_utils.o \
  src/agent/agent_password_test_dynamic_resource_scheduler.o \
  src/gssh/agent_password_test_agent_password_test.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent_password_test[0m']"
	$(CXX) src/agent_password_test_flags.o \
  common/agent_password_test_logging.o \
  common/agent_password_test_util.o \
  src/proto/agent_password_test_task.pb.o \
  src/proto/agent_password_test_agent.pb.o \
  src/proto/agent_password_test_master.pb.o \
  src/agent/agent_password_test_task_runner.o \
  src/agent/agent_password_test_task_manager.o \
  src/agent/agent_password_test_cgroup.o \
  src/agent/agent_password_test_workspace.o \
  src/agent/agent_password_test_workspace_manager.o \
  src/agent/agent_password_test_agent_impl.o \
  src/agent/agent_password_test_downloader_manager.o \
  src/agent/agent_password_test_curl_downloader.o \
  src/agent/agent_password_test_binary_downloader.o \
  src/agent/agent_password_test_resource_collector.o \
  src/agent/agent_password_test_resource_manager.o \
  src/agent/agent_password_test_resource_collector_engine.o \
  src/agent/agent_password_test_utils.o \
  src/agent/agent_password_test_dynamic_resource_scheduler.o \
  src/gssh/agent_password_test_agent_password_test.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o agent_password_test
	mkdir -p ./output/bin
	cp -f --link agent_password_test ./output/bin

gssh_password:src/proto/gssh_password_agent.pb.o \
  common/gssh_password_logging.o \
  src/gssh/gssh_password_gssh_password.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mgssh_password[0m']"
	$(CXX) src/proto/gssh_password_agent.pb.o \
  common/gssh_password_logging.o \
  src/gssh/gssh_password_gssh_password.o -Xlinker "-("  ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
  ../../../third-64/boost/lib/libboost_atomic.a \
  ../../../third-64/boost/lib/libboost_chrono.a \
  ../../../third-64/boost/lib/libboost_container.a \
  ../../../third-64/boost/lib/libboost_context.a \
  ../../../third-64/boost/lib/libboost_coroutine.a \
  ../../../third-64/boost/lib/libboost_date_time.a \
  ../../../third-64/boost/lib/libboost_exception.a \
  ../../../third-64/boost/lib/libboost_filesystem.a \
  ../../../third-64/boost/lib/libboost_graph.a \
  ../../../third-64/boost/lib/libboost_iostreams.a \
  ../../../third-64/boost/lib/libboost_locale.a \
  ../../../third-64/boost/lib/libboost_log.a \
  ../../../third-64/boost/lib/libboost_log_setup.a \
  ../../../third-64/boost/lib/libboost_math_c99.a \
  ../../../third-64/boost/lib/libboost_math_c99f.a \
  ../../../third-64/boost/lib/libboost_math_c99l.a \
  ../../../third-64/boost/lib/libboost_math_tr1.a \
  ../../../third-64/boost/lib/libboost_math_tr1f.a \
  ../../../third-64/boost/lib/libboost_math_tr1l.a \
  ../../../third-64/boost/lib/libboost_prg_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_program_options.a \
  ../../../third-64/boost/lib/libboost_python.a \
  ../../../third-64/boost/lib/libboost_random.a \
  ../../../third-64/boost/lib/libboost_regex.a \
  ../../../third-64/boost/lib/libboost_serialization.a \
  ../../../third-64/boost/lib/libboost_signals.a \
  ../../../third-64/boost/lib/libboost_system.a \
  ../../../third-64/boost/lib/libboost_test_exec_monitor.a \
  ../../../third-64/boost/lib/libboost_thread.a \
  ../../../third-64/boost/lib/libboost_timer.a \
  ../../../third-64/boost/lib/libboost_unit_test_framework.a \
  ../../../third-64/boost/lib/libboost_wave.a \
  ../../../third-64/boost/lib/libboost_wserialization.a \
  ../../../third-64/gflags/lib/libgflags.a \
  ../../../third-64/gflags/lib/libgflags_nothreads.a \
  ../../../third-64/gtest/lib/libgtest.a \
  ../../../third-64/gtest/lib/libgtest_main.a \
  ../../../third-64/leveldb/lib/libleveldb.a \
  ../../../third-64/libcurl/lib/libcurl.a \
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o gssh_password
	mkdir -p ./output/bin
	cp -f --link gssh_password ./output/bin

libpam_galaxy.so:src/gssh/pam_galaxy_pam_galaxy.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibpam_galaxy.so[0m']"
	$(CC) -shared src/gssh/pam_galaxy_pam_galaxy.o -Xlinker "-("  -lpthread \
  -lcrypto \
  -lrt \
  -lidn \
  -lssl \
  -lldap \
  -lboost_regex -Xlinker "-)" -o libpam_galaxy.so
	mkdir -p ./output/so
	cp -f --link libpam_galaxy.so ./output/so

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

src/proto/monitor.pb.cc \
  src/proto/monitor.pb.h:src/proto/monitor.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/monitor.pb.cc \
  src/proto/monitor.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/monitor.proto

src/proto/monitor.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/monitor.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/agent_monitor.pb.o:src/proto/monitor.pb.cc \
  src/proto/monitor.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_monitor.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_monitor.pb.o src/proto/monitor.pb.cc

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
  src/agent/resource_collector.h \
  common/logging.h \
  common/util.h \
  common/this_thread.h \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector_engine.h \
  common/thread_pool.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_task_runner.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_task_runner.o src/agent/task_runner.cc

src/agent/agent_task_manager.o:src/agent/task_manager.cc \
  src/agent/task_manager.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/task_runner.h \
  src/agent/workspace.h \
  src/agent/resource_collector.h \
  common/logging.h \
  src/agent/cgroup.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_task_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_task_manager.o src/agent/task_manager.cc

src/agent/agent_cgroup.o:src/agent/cgroup.cc \
  src/agent/cgroup.h \
  src/agent/task_runner.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/workspace.h \
  src/agent/resource_collector.h \
  common/logging.h \
  common/util.h \
  common/this_thread.h \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/agent/resource_collector_engine.h \
  src/agent/utils.h \
  src/agent/dynamic_resource_scheduler.h \
  src/agent/resource_manager.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_cgroup.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_cgroup.o src/agent/cgroup.cc

src/agent/agent_workspace.o:src/agent/workspace.cc \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/logging.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_workspace.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_workspace.o src/agent/workspace.cc

src/agent/agent_workspace_manager.o:src/agent/workspace_manager.cc \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  common/logging.h \
  src/agent/utils.h
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
  common/thread_pool.h \
  common/mutex.h \
  src/agent/task_manager.h \
  src/agent/task_runner.h \
  src/agent/resource_collector.h \
  src/agent/resource_manager.h \
  common/thread_pool.h \
  src/agent/workspace.h \
  common/util.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/thread_pool.h \
  common/logging.h \
  src/proto/task.pb.h \
  common/httpserver.h \
  common/logging.h \
  common/thread_pool.h \
  src/agent/dynamic_resource_scheduler.h \
  src/agent/resource_manager.h
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
  common/thread_pool.h \
  common/mutex.h \
  src/agent/task_manager.h \
  src/agent/task_runner.h \
  src/agent/resource_collector.h \
  src/agent/resource_manager.h \
  common/thread_pool.h \
  src/agent/workspace.h \
  src/agent/resource_collector_engine.h \
  src/agent/dynamic_resource_scheduler.h \
  src/agent/resource_manager.h
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

src/agent/agent_resource_collector.o:src/agent/resource_collector.cc \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h \
  common/mutex.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_resource_collector.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_resource_collector.o src/agent/resource_collector.cc

src/agent/agent_resource_manager.o:src/agent/resource_manager.cc \
  src/agent/resource_manager.h \
  common/mutex.h \
  common/timer.h \
  common/logging.h \
  src/agent/dynamic_resource_scheduler.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  src/agent/resource_manager.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_resource_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_resource_manager.o src/agent/resource_manager.cc

src/agent/agent_resource_collector_engine.o:src/agent/resource_collector_engine.cc \
  src/agent/resource_collector_engine.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_resource_collector_engine.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_resource_collector_engine.o src/agent/resource_collector_engine.cc

src/agent/agent_utils.o:src/agent/utils.cc \
  src/agent/utils.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_utils.o src/agent/utils.cc

src/agent/agent_dynamic_resource_scheduler.o:src/agent/dynamic_resource_scheduler.cc \
  src/agent/dynamic_resource_scheduler.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  src/agent/resource_manager.h \
  src/agent/resource_collector_engine.h \
  common/logging.h \
  common/util.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_dynamic_resource_scheduler.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_dynamic_resource_scheduler.o src/agent/dynamic_resource_scheduler.cc

src/master_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master_flags.o src/flags.cc

src/proto/master_monitor.pb.o:src/proto/monitor.pb.cc \
  src/proto/monitor.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_monitor.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_monitor.pb.o src/proto/monitor.pb.cc

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
  src/proto/agent.pb.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/thread_pool.h \
  common/logging.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_impl.o src/master/master_impl.cc

src/master/master_master_main.o:src/master/master_main.cc \
  src/master/master_impl.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/proto/agent.pb.h \
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

src/curl_downloader_test_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/curl_downloader_test_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/curl_downloader_test_flags.o src/flags.cc

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

src/downloader_mgr_test_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/downloader_mgr_test_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/downloader_mgr_test_flags.o src/flags.cc

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

src/agent/resource_collector_test_resource_collector.o:src/agent/resource_collector.cc \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h \
  common/mutex.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/resource_collector_test_resource_collector.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/resource_collector_test_resource_collector.o src/agent/resource_collector.cc

test/resource_collector_test_resource_collector_test.o:test/resource_collector_test.cc \
  src/agent/resource_collector_engine.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/resource_collector_test_resource_collector_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/resource_collector_test_resource_collector_test.o test/resource_collector_test.cc

common/resource_collector_test_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/resource_collector_test_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/resource_collector_test_logging.o common/logging.cc

src/agent/resource_collector_test_resource_collector_engine.o:src/agent/resource_collector_engine.cc \
  src/agent/resource_collector_engine.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/resource_collector_test_resource_collector_engine.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/resource_collector_test_resource_collector_engine.o src/agent/resource_collector_engine.cc

src/resource_collector_test_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/resource_collector_test_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/resource_collector_test_flags.o src/flags.cc

src/agent/agent_utils_test_utils.o:src/agent/utils.cc \
  src/agent/utils.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_utils_test_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_utils_test_utils.o src/agent/utils.cc

common/agent_utils_test_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_utils_test_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_utils_test_logging.o common/logging.cc

test/agent_utils_test_agent_utils_test.o:test/agent_utils_test.cc \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/agent_utils_test_agent_utils_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/agent_utils_test_agent_utils_test.o test/agent_utils_test.cc

src/agent/agent_utils_remove_test_utils.o:src/agent/utils.cc \
  src/agent/utils.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_utils_remove_test_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_utils_remove_test_utils.o src/agent/utils.cc

common/agent_utils_remove_test_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_utils_remove_test_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_utils_remove_test_logging.o common/logging.cc

test/agent_utils_remove_test_agent_utils_remove_test.o:test/agent_utils_remove_test.cc \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/agent_utils_remove_test_agent_utils_remove_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/agent_utils_remove_test_agent_utils_remove_test.o test/agent_utils_remove_test.cc

src/proto/agent_mock_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_mock_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_mock_master.pb.o src/proto/master.pb.cc

common/agent_mock_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_mock_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_mock_logging.o common/logging.cc

src/proto/agent_mock_task.pb.o:src/proto/task.pb.cc \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_mock_task.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_mock_task.pb.o src/proto/task.pb.cc

src/proto/agent_mock_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_mock_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_mock_agent.pb.o src/proto/agent.pb.cc

test/agent_mock_pb_file_parser.o:test/pb_file_parser.cc \
  test/pb_file_parser.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/agent_mock_pb_file_parser.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/agent_mock_pb_file_parser.o test/pb_file_parser.cc

test/agent_mock_agent_test_mock.o:test/agent_test_mock.cc \
  test/pb_file_parser.h \
  src/proto/agent.pb.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/agent_mock_agent_test_mock.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/agent_mock_agent_test_mock.o test/agent_test_mock.cc

src/proto/master_mock_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_mock_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_mock_master.pb.o src/proto/master.pb.cc

src/proto/master_mock_task.pb.o:src/proto/task.pb.cc \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_mock_task.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_mock_task.pb.o src/proto/task.pb.cc

test/master_mock_pb_file_parser.o:test/pb_file_parser.cc \
  test/pb_file_parser.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/master_mock_pb_file_parser.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/master_mock_pb_file_parser.o test/pb_file_parser.cc

test/master_mock_master_test_mock.o:test/master_test_mock.cc \
  test/pb_file_parser.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest/master_mock_master_test_mock.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test/master_mock_master_test_mock.o test/master_test_mock.cc

common/monitor_agent_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/monitor_agent_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/monitor_agent_logging.o common/logging.cc

common/monitor_agent_util.o:common/util.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/monitor_agent_util.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/monitor_agent_util.o common/util.cc

src/monitor_agent_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/monitor_agent_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/monitor_agent_flags.o src/flags.cc

src/monitor/monitor_agent_monitor_main.o:src/monitor/monitor_main.cc \
  src/monitor/monitor_impl.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/monitor/monitor_agent_monitor_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/monitor/monitor_agent_monitor_main.o src/monitor/monitor_main.cc

src/monitor/monitor_agent_monitor_impl.o:src/monitor/monitor_impl.cc \
  common/asm_atomic.h \
  common/logging.h \
  common/util.h \
  src/proto/monitor.pb.h \
  src/monitor/monitor_impl.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/monitor/monitor_agent_monitor_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/monitor/monitor_agent_monitor_impl.o src/monitor/monitor_impl.cc

src/proto/monitor_agent_monitor.pb.o:src/proto/monitor.pb.cc \
  src/proto/monitor.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/monitor_agent_monitor.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/monitor_agent_monitor.pb.o src/proto/monitor.pb.cc

src/agent_password_test_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent_password_test_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent_password_test_flags.o src/flags.cc

common/agent_password_test_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_password_test_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_password_test_logging.o common/logging.cc

common/agent_password_test_util.o:common/util.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/agent_password_test_util.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/agent_password_test_util.o common/util.cc

src/proto/agent_password_test_task.pb.o:src/proto/task.pb.cc \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_password_test_task.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_password_test_task.pb.o src/proto/task.pb.cc

src/proto/agent_password_test_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_password_test_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_password_test_agent.pb.o src/proto/agent.pb.cc

src/proto/agent_password_test_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/task.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_password_test_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_password_test_master.pb.o src/proto/master.pb.cc

src/agent/agent_password_test_task_runner.o:src/agent/task_runner.cc \
  src/agent/task_runner.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/workspace.h \
  src/agent/resource_collector.h \
  common/logging.h \
  common/util.h \
  common/this_thread.h \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector_engine.h \
  common/thread_pool.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_task_runner.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_task_runner.o src/agent/task_runner.cc

src/agent/agent_password_test_task_manager.o:src/agent/task_manager.cc \
  src/agent/task_manager.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/task_runner.h \
  src/agent/workspace.h \
  src/agent/resource_collector.h \
  common/logging.h \
  src/agent/cgroup.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_task_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_task_manager.o src/agent/task_manager.cc

src/agent/agent_password_test_cgroup.o:src/agent/cgroup.cc \
  src/agent/cgroup.h \
  src/agent/task_runner.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  src/agent/workspace.h \
  src/agent/resource_collector.h \
  common/logging.h \
  common/util.h \
  common/this_thread.h \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/agent/resource_collector_engine.h \
  src/agent/utils.h \
  src/agent/dynamic_resource_scheduler.h \
  src/agent/resource_manager.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_cgroup.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_cgroup.o src/agent/cgroup.cc

src/agent/agent_password_test_workspace.o:src/agent/workspace.cc \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/logging.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_workspace.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_workspace.o src/agent/workspace.cc

src/agent/agent_password_test_workspace_manager.o:src/agent/workspace_manager.cc \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  common/logging.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_workspace_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_workspace_manager.o src/agent/workspace_manager.cc

src/agent/agent_password_test_agent_impl.o:src/agent/agent_impl.cc \
  src/agent/agent_impl.h \
  src/proto/agent.pb.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/task_manager.h \
  src/agent/task_runner.h \
  src/agent/resource_collector.h \
  src/agent/resource_manager.h \
  common/thread_pool.h \
  src/agent/workspace.h \
  common/util.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/thread_pool.h \
  common/logging.h \
  src/proto/task.pb.h \
  common/httpserver.h \
  common/logging.h \
  common/thread_pool.h \
  src/agent/dynamic_resource_scheduler.h \
  src/agent/resource_manager.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_agent_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_agent_impl.o src/agent/agent_impl.cc

src/agent/agent_password_test_downloader_manager.o:src/agent/downloader_manager.cc \
  src/agent/downloader_manager.h \
  src/agent/downloader.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/asm_atomic.h \
  src/agent/curl_downloader.h \
  src/agent/binary_downloader.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_downloader_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_downloader_manager.o src/agent/downloader_manager.cc

src/agent/agent_password_test_curl_downloader.o:src/agent/curl_downloader.cc \
  src/agent/curl_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_curl_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_curl_downloader.o src/agent/curl_downloader.cc

src/agent/agent_password_test_binary_downloader.o:src/agent/binary_downloader.cc \
  src/agent/binary_downloader.h \
  src/agent/downloader.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_binary_downloader.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_binary_downloader.o src/agent/binary_downloader.cc

src/agent/agent_password_test_resource_collector.o:src/agent/resource_collector.cc \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h \
  common/mutex.h \
  common/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_resource_collector.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_resource_collector.o src/agent/resource_collector.cc

src/agent/agent_password_test_resource_manager.o:src/agent/resource_manager.cc \
  src/agent/resource_manager.h \
  common/mutex.h \
  common/timer.h \
  common/logging.h \
  src/agent/dynamic_resource_scheduler.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  src/agent/resource_manager.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_resource_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_resource_manager.o src/agent/resource_manager.cc

src/agent/agent_password_test_resource_collector_engine.o:src/agent/resource_collector_engine.cc \
  src/agent/resource_collector_engine.h \
  common/thread_pool.h \
  common/mutex.h \
  common/timer.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_resource_collector_engine.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_resource_collector_engine.o src/agent/resource_collector_engine.cc

src/agent/agent_password_test_utils.o:src/agent/utils.cc \
  src/agent/utils.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_utils.o src/agent/utils.cc

src/agent/agent_password_test_dynamic_resource_scheduler.o:src/agent/dynamic_resource_scheduler.cc \
  src/agent/dynamic_resource_scheduler.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  src/agent/resource_collector.h \
  src/proto/task.pb.h \
  src/agent/resource_manager.h \
  src/agent/resource_collector_engine.h \
  common/logging.h \
  common/util.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_password_test_dynamic_resource_scheduler.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_password_test_dynamic_resource_scheduler.o src/agent/dynamic_resource_scheduler.cc

src/gssh/agent_password_test_agent_password_test.o:src/gssh/agent_password_test.cc \
  src/proto/agent.pb.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  common/logging.h \
  src/agent/agent_impl.h \
  src/proto/agent.pb.h \
  src/proto/master.pb.h \
  src/proto/task.pb.h \
  src/agent/workspace_manager.h \
  src/agent/workspace.h \
  src/proto/task.pb.h \
  common/mutex.h \
  common/thread_pool.h \
  src/agent/task_manager.h \
  src/agent/task_runner.h \
  src/agent/resource_collector.h \
  src/agent/resource_manager.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gssh/agent_password_test_agent_password_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gssh/agent_password_test_agent_password_test.o src/gssh/agent_password_test.cc

src/proto/gssh_password_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gssh_password_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/gssh_password_agent.pb.o src/proto/agent.pb.cc

common/gssh_password_logging.o:common/logging.cc \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcommon/gssh_password_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o common/gssh_password_logging.o common/logging.cc

src/gssh/gssh_password_gssh_password.o:src/gssh/gssh_password.cc \
  src/proto/agent.pb.h \
  src/rpc/rpc_client.h \
  common/mutex.h \
  common/timer.h \
  common/thread_pool.h \
  common/mutex.h \
  common/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gssh/gssh_password_gssh_password.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gssh/gssh_password_gssh_password.o src/gssh/gssh_password.cc

src/gssh/pam_galaxy_pam_galaxy.o:src/gssh/pam_galaxy.c
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gssh/pam_galaxy_pam_galaxy.o[0m']"
	$(CC) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CFLAGS)  -o src/gssh/pam_galaxy_pam_galaxy.o src/gssh/pam_galaxy.c

endif #ifeq ($(shell uname -m),x86_64)


