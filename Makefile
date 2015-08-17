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
  -I./output/include \
  -I./common/include \
  -I./ins/sdk/
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
  -I../../../third-64/protobuf \
  -I../../../third-64/protobuf/include \
  -I../../../third-64/protobuf/output \
  -I../../../third-64/protobuf/output/include \
  -I../../../third-64/snappy \
  -I../../../third-64/snappy/include \
  -I../../../third-64/snappy/output \
  -I../../../third-64/snappy/output/include \
  -I../../../thirdsrc/protobuf \
  -I../../../thirdsrc/protobuf/include \
  -I../../../thirdsrc/protobuf/output \
  -I../../../thirdsrc/protobuf/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=183c4ecf12ea1279802ad73207134d05  COMAKE


.PHONY:all
all:comake2_makefile_check scheduler master agent initd gced libgalaxy.a galaxy 
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
	rm -rf scheduler
	rm -rf ./output/bin/scheduler
	rm -rf master
	rm -rf ./output/bin/master
	rm -rf agent
	rm -rf ./output/bin/agent
	rm -rf initd
	rm -rf ./output/bin/initd
	rm -rf gced
	rm -rf ./output/bin/gced
	rm -rf libgalaxy.a
	rm -rf ./output/lib/libgalaxy.a
	rm -rf ./output/include/galaxy.h
	rm -rf galaxy
	rm -rf ./output/bin/galaxy
	rm -rf src/scheduler/scheduler_scheduler.o
	rm -rf src/proto/galaxy.pb.cc
	rm -rf src/proto/galaxy.pb.h
	rm -rf src/proto/scheduler_galaxy.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/scheduler_master.pb.o
	rm -rf src/scheduler/scheduler_scheduler_io.o
	rm -rf src/utils/scheduler_resource_utils.o
	rm -rf src/scheduler_flags.o
	rm -rf src/master/scheduler_master_watcher.o
	rm -rf src/scheduler/scheduler_scheduler_main.o
	rm -rf src/master/master_master_impl.o
	rm -rf src/master/master_master_util.o
	rm -rf src/master/master_master_main.o
	rm -rf src/master/master_job_manager.o
	rm -rf src/master_flags.o
	rm -rf src/master/master_master_watcher.o
	rm -rf src/utils/master_resource_utils.o
	rm -rf src/proto/galaxy.pb.cc
	rm -rf src/proto/galaxy.pb.h
	rm -rf src/proto/master_galaxy.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/master_master.pb.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/master_agent.pb.o
	rm -rf src/agent/agent_agent_impl.o
	rm -rf src/agent/agent_agent_main.o
	rm -rf src/agent/agent_pod_manager.o
	rm -rf src/agent/agent_initd_handler.o
	rm -rf src/agent/agent_utils.o
	rm -rf src/agent_flags.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/agent_agent.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/agent_master.pb.o
	rm -rf src/master/agent_master_watcher.o
	rm -rf src/proto/galaxy.pb.cc
	rm -rf src/proto/galaxy.pb.h
	rm -rf src/proto/agent_galaxy.pb.o
	rm -rf src/proto/gced.pb.cc
	rm -rf src/proto/gced.pb.h
	rm -rf src/proto/agent_gced.pb.o
	rm -rf src/proto/initd.pb.cc
	rm -rf src/proto/initd.pb.h
	rm -rf src/proto/agent_initd.pb.o
	rm -rf src/gce/initd_initd_main.o
	rm -rf src/gce/initd_initd_impl.o
	rm -rf src/gce/initd_utils.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/initd_agent.pb.o
	rm -rf src/proto/galaxy.pb.cc
	rm -rf src/proto/galaxy.pb.h
	rm -rf src/proto/initd_galaxy.pb.o
	rm -rf src/initd_flags.o
	rm -rf src/proto/initd.pb.cc
	rm -rf src/proto/initd.pb.h
	rm -rf src/proto/initd_initd.pb.o
	rm -rf src/gce/gced_gced_impl.o
	rm -rf src/gce/gced_gced_main.o
	rm -rf src/gced_flags.o
	rm -rf src/gce/gced_utils.o
	rm -rf src/proto/gced.pb.cc
	rm -rf src/proto/gced.pb.h
	rm -rf src/proto/gced_gced.pb.o
	rm -rf src/proto/agent.pb.cc
	rm -rf src/proto/agent.pb.h
	rm -rf src/proto/gced_agent.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/gced_master.pb.o
	rm -rf src/proto/galaxy.pb.cc
	rm -rf src/proto/galaxy.pb.h
	rm -rf src/proto/gced_galaxy.pb.o
	rm -rf src/proto/initd.pb.cc
	rm -rf src/proto/initd.pb.h
	rm -rf src/proto/gced_initd.pb.o
	rm -rf src/sdk/galaxy_galaxy.o
	rm -rf src/proto/galaxy.pb.cc
	rm -rf src/proto/galaxy.pb.h
	rm -rf src/proto/galaxy_galaxy.pb.o
	rm -rf src/proto/master.pb.cc
	rm -rf src/proto/master.pb.h
	rm -rf src/proto/galaxy_master.pb.o
	rm -rf src/client/galaxy_galaxy_client.o
	rm -rf src/galaxy_flags.o

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

scheduler:src/scheduler/scheduler_scheduler.o \
  src/proto/scheduler_galaxy.pb.o \
  src/proto/scheduler_master.pb.o \
  src/scheduler/scheduler_scheduler_io.o \
  src/utils/scheduler_resource_utils.o \
  src/scheduler_flags.o \
  src/master/scheduler_master_watcher.o \
  src/scheduler/scheduler_scheduler_main.o \
  ./common/libcommon.a \
  ./ins/libins_sdk.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mscheduler[0m']"
	$(CXX) src/scheduler/scheduler_scheduler.o \
  src/proto/scheduler_galaxy.pb.o \
  src/proto/scheduler_master.pb.o \
  src/scheduler/scheduler_scheduler_io.o \
  src/utils/scheduler_resource_utils.o \
  src/scheduler_flags.o \
  src/master/scheduler_master_watcher.o \
  src/scheduler/scheduler_scheduler_main.o -Xlinker "-(" ./common/libcommon.a \
  ./ins/libins_sdk.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
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
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf-lite.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf.a \
  ../../../thirdsrc/protobuf/output/lib/libprotoc.a -lpthread \
  -lz -Xlinker "-)" -o scheduler
	mkdir -p ./output/bin
	cp -f --link scheduler ./output/bin

master:src/master/master_master_impl.o \
  src/master/master_master_util.o \
  src/master/master_master_main.o \
  src/master/master_job_manager.o \
  src/master_flags.o \
  src/master/master_master_watcher.o \
  src/utils/master_resource_utils.o \
  src/proto/master_galaxy.pb.o \
  src/proto/master_master.pb.o \
  src/proto/master_agent.pb.o \
  ./common/libcommon.a \
  ./ins/libins_sdk.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mmaster[0m']"
	$(CXX) src/master/master_master_impl.o \
  src/master/master_master_util.o \
  src/master/master_master_main.o \
  src/master/master_job_manager.o \
  src/master_flags.o \
  src/master/master_master_watcher.o \
  src/utils/master_resource_utils.o \
  src/proto/master_galaxy.pb.o \
  src/proto/master_master.pb.o \
  src/proto/master_agent.pb.o -Xlinker "-(" ./common/libcommon.a \
  ./ins/libins_sdk.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
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
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf-lite.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf.a \
  ../../../thirdsrc/protobuf/output/lib/libprotoc.a -lpthread \
  -lz -Xlinker "-)" -o master
	mkdir -p ./output/bin
	cp -f --link master ./output/bin

agent:src/agent/agent_agent_impl.o \
  src/agent/agent_agent_main.o \
  src/agent/agent_pod_manager.o \
  src/agent/agent_initd_handler.o \
  src/agent/agent_utils.o \
  src/agent_flags.o \
  src/proto/agent_agent.pb.o \
  src/proto/agent_master.pb.o \
  src/master/agent_master_watcher.o \
  src/proto/agent_galaxy.pb.o \
  src/proto/agent_gced.pb.o \
  src/proto/agent_initd.pb.o \
  ./common/libcommon.a \
  ./ins/libins_sdk.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40magent[0m']"
	$(CXX) src/agent/agent_agent_impl.o \
  src/agent/agent_agent_main.o \
  src/agent/agent_pod_manager.o \
  src/agent/agent_initd_handler.o \
  src/agent/agent_utils.o \
  src/agent_flags.o \
  src/proto/agent_agent.pb.o \
  src/proto/agent_master.pb.o \
  src/master/agent_master_watcher.o \
  src/proto/agent_galaxy.pb.o \
  src/proto/agent_gced.pb.o \
  src/proto/agent_initd.pb.o -Xlinker "-(" ./common/libcommon.a \
  ./ins/libins_sdk.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
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
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf-lite.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf.a \
  ../../../thirdsrc/protobuf/output/lib/libprotoc.a -lpthread \
  -lz -Xlinker "-)" -o agent
	mkdir -p ./output/bin
	cp -f --link agent ./output/bin

initd:src/gce/initd_initd_main.o \
  src/gce/initd_initd_impl.o \
  src/gce/initd_utils.o \
  src/proto/initd_agent.pb.o \
  src/proto/initd_galaxy.pb.o \
  src/initd_flags.o \
  src/proto/initd_initd.pb.o \
  ./common/libcommon.a \
  ./ins/libins_sdk.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minitd[0m']"
	$(CXX) src/gce/initd_initd_main.o \
  src/gce/initd_initd_impl.o \
  src/gce/initd_utils.o \
  src/proto/initd_agent.pb.o \
  src/proto/initd_galaxy.pb.o \
  src/initd_flags.o \
  src/proto/initd_initd.pb.o -Xlinker "-(" ./common/libcommon.a \
  ./ins/libins_sdk.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
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
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf-lite.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf.a \
  ../../../thirdsrc/protobuf/output/lib/libprotoc.a -lpthread \
  -lz -Xlinker "-)" -o initd
	mkdir -p ./output/bin
	cp -f --link initd ./output/bin

gced:src/gce/gced_gced_impl.o \
  src/gce/gced_gced_main.o \
  src/gced_flags.o \
  src/gce/gced_utils.o \
  src/proto/gced_gced.pb.o \
  src/proto/gced_agent.pb.o \
  src/proto/gced_master.pb.o \
  src/proto/gced_galaxy.pb.o \
  src/proto/gced_initd.pb.o \
  ./common/libcommon.a \
  ./ins/libins_sdk.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mgced[0m']"
	$(CXX) src/gce/gced_gced_impl.o \
  src/gce/gced_gced_main.o \
  src/gced_flags.o \
  src/gce/gced_utils.o \
  src/proto/gced_gced.pb.o \
  src/proto/gced_agent.pb.o \
  src/proto/gced_master.pb.o \
  src/proto/gced_galaxy.pb.o \
  src/proto/gced_initd.pb.o -Xlinker "-(" ./common/libcommon.a \
  ./ins/libins_sdk.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
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
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf-lite.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf.a \
  ../../../thirdsrc/protobuf/output/lib/libprotoc.a -lpthread \
  -lz -Xlinker "-)" -o gced
	mkdir -p ./output/bin
	cp -f --link gced ./output/bin

libgalaxy.a:src/sdk/galaxy_galaxy.o \
  src/proto/galaxy_galaxy.pb.o \
  src/proto/galaxy_master.pb.o \
  src/sdk/galaxy.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibgalaxy.a[0m']"
	ar crs libgalaxy.a src/sdk/galaxy_galaxy.o \
  src/proto/galaxy_galaxy.pb.o \
  src/proto/galaxy_master.pb.o
	mkdir -p ./output/lib
	cp -f --link libgalaxy.a ./output/lib
	mkdir -p ./output/include
	cp -f --link src/sdk/galaxy.h ./output/include

galaxy:src/client/galaxy_galaxy_client.o \
  src/galaxy_flags.o \
  ./libgalaxy.a \
  ./common/libcommon.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mgalaxy[0m']"
	$(CXX) src/client/galaxy_galaxy_client.o \
  src/galaxy_flags.o -Xlinker "-(" ./libgalaxy.a \
  ./common/libcommon.a ../../../public/sofa-pbrpc/libsofa-pbrpc.a \
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
  ../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../third-64/protobuf/lib/libprotoc.a \
  ../../../third-64/snappy/lib/libsnappy.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf-lite.a \
  ../../../thirdsrc/protobuf/output/lib/libprotobuf.a \
  ../../../thirdsrc/protobuf/output/lib/libprotoc.a -lpthread \
  -lz -Xlinker "-)" -o galaxy
	mkdir -p ./output/bin
	cp -f --link galaxy ./output/bin

src/scheduler/scheduler_scheduler.o:src/scheduler/scheduler.cc \
  src/scheduler/scheduler.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/logging.h \
  src/utils/resource_utils.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/scheduler/scheduler_scheduler.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/scheduler/scheduler_scheduler.o src/scheduler/scheduler.cc

src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h:src/proto/galaxy.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/galaxy.proto

src/proto/galaxy.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/galaxy.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/scheduler_galaxy.pb.o:src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/scheduler_galaxy.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/scheduler_galaxy.pb.o src/proto/galaxy.pb.cc

src/proto/master.pb.cc \
  src/proto/master.pb.h:src/proto/master.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master.pb.cc \
  src/proto/master.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/master.proto

src/proto/master.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/scheduler_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/scheduler_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/scheduler_master.pb.o src/proto/master.pb.cc

src/scheduler/scheduler_scheduler_io.o:src/scheduler/scheduler_io.cc \
  src/scheduler/scheduler_io.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  src/scheduler/scheduler.h \
  common/include/mutex.h \
  common/include/timer.h \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h \
  common/include/mutex.h \
  src/rpc/rpc_client.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/scheduler/scheduler_scheduler_io.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/scheduler/scheduler_scheduler_io.o src/scheduler/scheduler_io.cc

src/utils/scheduler_resource_utils.o:src/utils/resource_utils.cc \
  src/utils/resource_utils.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/utils/scheduler_resource_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/utils/scheduler_resource_utils.o src/utils/resource_utils.cc

src/scheduler_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/scheduler_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/scheduler_flags.o src/flags.cc

src/master/scheduler_master_watcher.o:src/master/master_watcher.cc \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/scheduler_master_watcher.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/scheduler_master_watcher.o src/master/master_watcher.cc

src/scheduler/scheduler_scheduler_main.o:src/scheduler/scheduler_main.cc \
  src/scheduler/scheduler_io.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  src/scheduler/scheduler.h \
  common/include/mutex.h \
  common/include/timer.h \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h \
  common/include/mutex.h \
  src/rpc/rpc_client.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/scheduler/scheduler_scheduler_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/scheduler/scheduler_scheduler_main.o src/scheduler/scheduler_main.cc

src/master/master_master_impl.o:src/master/master_impl.cc \
  src/master/master_impl.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  src/master/job_manager.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/timer.h \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/logging.h \
  ins/sdk/ins_sdk.h \
  src/master/master_util.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_impl.o src/master/master_impl.cc

src/master/master_master_util.o:src/master/master_util.cc \
  src/master/master_util.h \
  common/include/logging.h \
  src/proto/galaxy.pb.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_util.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_util.o src/master/master_util.cc

src/master/master_master_main.o:src/master/master_main.cc \
  common/include/logging.h \
  src/master/master_impl.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  src/master/job_manager.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/timer.h \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  ins/sdk/ins_sdk.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_main.o src/master/master_main.cc

src/master/master_job_manager.o:src/master/job_manager.cc \
  src/master/job_manager.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/timer.h \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/logging.h \
  src/master/master_util.h \
  src/utils/resource_utils.h \
  src/proto/galaxy.pb.h \
  common/include/timer.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_job_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_job_manager.o src/master/job_manager.cc

src/master_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master_flags.o src/flags.cc

src/master/master_master_watcher.o:src/master/master_watcher.cc \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/master_master_watcher.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/master_master_watcher.o src/master/master_watcher.cc

src/utils/master_resource_utils.o:src/utils/resource_utils.cc \
  src/utils/resource_utils.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/utils/master_resource_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/utils/master_resource_utils.o src/utils/resource_utils.cc

src/proto/master_galaxy.pb.o:src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_galaxy.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_galaxy.pb.o src/proto/galaxy.pb.cc

src/proto/master_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_master.pb.o src/proto/master.pb.cc

src/proto/agent.pb.cc \
  src/proto/agent.pb.h:src/proto/agent.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent.pb.cc \
  src/proto/agent.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/agent.proto

src/proto/agent.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/master_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/master_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/master_agent.pb.o src/proto/agent.pb.cc

src/agent/agent_agent_impl.o:src/agent/agent_impl.cc \
  src/agent/agent_impl.h \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h \
  src/proto/master.pb.h \
  src/proto/gced.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/thread_pool.h \
  common/include/logging.h \
  src/agent/pod_manager.h \
  src/agent/pod_info.h \
  src/proto/galaxy.pb.h \
  src/agent/initd_handler.h \
  src/proto/initd.pb.h \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h \
  src/proto/master.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_agent_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_agent_impl.o src/agent/agent_impl.cc

src/agent/agent_agent_main.o:src/agent/agent_main.cc \
  src/agent/agent_impl.h \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h \
  src/proto/master.pb.h \
  src/proto/gced.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/thread_pool.h \
  common/include/logging.h \
  src/agent/pod_manager.h \
  src/agent/pod_info.h \
  src/proto/galaxy.pb.h \
  src/agent/initd_handler.h \
  src/proto/initd.pb.h \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_agent_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_agent_main.o src/agent/agent_main.cc

src/agent/agent_pod_manager.o:src/agent/pod_manager.cc \
  src/agent/pod_manager.h \
  src/agent/pod_info.h \
  src/proto/galaxy.pb.h \
  src/agent/initd_handler.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h \
  src/proto/galaxy.pb.h \
  common/include/thread.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_pod_manager.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_pod_manager.o src/agent/pod_manager.cc

src/agent/agent_initd_handler.o:src/agent/initd_handler.cc \
  src/agent/initd_handler.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h \
  src/agent/pod_info.h \
  src/proto/galaxy.pb.h \
  src/agent/pod_info.h \
  common/include/thread.h \
  src/agent/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_initd_handler.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_initd_handler.o src/agent/initd_handler.cc

src/agent/agent_utils.o:src/agent/utils.cc \
  src/gce/utils.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent/agent_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent/agent_utils.o src/agent/utils.cc

src/agent_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/agent_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/agent_flags.o src/flags.cc

src/proto/agent_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_agent.pb.o src/proto/agent.pb.cc

src/proto/agent_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_master.pb.o src/proto/master.pb.cc

src/master/agent_master_watcher.o:src/master/master_watcher.cc \
  src/master/master_watcher.h \
  ins/sdk/ins_sdk.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/master/agent_master_watcher.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/master/agent_master_watcher.o src/master/master_watcher.cc

src/proto/agent_galaxy.pb.o:src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_galaxy.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_galaxy.pb.o src/proto/galaxy.pb.cc

src/proto/gced.pb.cc \
  src/proto/gced.pb.h:src/proto/gced.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced.pb.cc \
  src/proto/gced.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/gced.proto

src/proto/gced.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/agent_gced.pb.o:src/proto/gced.pb.cc \
  src/proto/gced.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_gced.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_gced.pb.o src/proto/gced.pb.cc

src/proto/initd.pb.cc \
  src/proto/initd.pb.h:src/proto/initd.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/initd.pb.cc \
  src/proto/initd.pb.h[0m']"
	../../../third-64/protobuf/bin/protoc --cpp_out=src/proto --proto_path=src/proto  src/proto/initd.proto

src/proto/initd.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/initd.proto[0m']"
	@echo "ALREADY BUILT"

src/proto/agent_initd.pb.o:src/proto/initd.pb.cc \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/agent_initd.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/agent_initd.pb.o src/proto/initd.pb.cc

src/gce/initd_initd_main.o:src/gce/initd_main.cc \
  src/gce/initd_impl.h \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h \
  src/gce/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gce/initd_initd_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gce/initd_initd_main.o src/gce/initd_main.cc

src/gce/initd_initd_impl.o:src/gce/initd_impl.cc \
  src/gce/initd_impl.h \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  src/gce/utils.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gce/initd_initd_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gce/initd_initd_impl.o src/gce/initd_impl.cc

src/gce/initd_utils.o:src/gce/utils.cc \
  src/gce/utils.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gce/initd_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gce/initd_utils.o src/gce/utils.cc

src/proto/initd_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/initd_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/initd_agent.pb.o src/proto/agent.pb.cc

src/proto/initd_galaxy.pb.o:src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/initd_galaxy.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/initd_galaxy.pb.o src/proto/galaxy.pb.cc

src/initd_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/initd_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/initd_flags.o src/flags.cc

src/proto/initd_initd.pb.o:src/proto/initd.pb.cc \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/initd_initd.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/initd_initd.pb.o src/proto/initd.pb.cc

src/gce/gced_gced_impl.o:src/gce/gced_impl.cc \
  src/gce/gced_impl.h \
  src/proto/gced.pb.h \
  src/proto/galaxy.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h \
  src/proto/initd.pb.h \
  src/gce/utils.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gce/gced_gced_impl.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gce/gced_gced_impl.o src/gce/gced_impl.cc

src/gce/gced_gced_main.o:src/gce/gced_main.cc \
  src/gce/gced_impl.h \
  src/proto/gced.pb.h \
  src/proto/galaxy.pb.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gce/gced_gced_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gce/gced_gced_main.o src/gce/gced_main.cc

src/gced_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gced_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gced_flags.o src/flags.cc

src/gce/gced_utils.o:src/gce/utils.cc \
  src/gce/utils.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/gce/gced_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/gce/gced_utils.o src/gce/utils.cc

src/proto/gced_gced.pb.o:src/proto/gced.pb.cc \
  src/proto/gced.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced_gced.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/gced_gced.pb.o src/proto/gced.pb.cc

src/proto/gced_agent.pb.o:src/proto/agent.pb.cc \
  src/proto/agent.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced_agent.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/gced_agent.pb.o src/proto/agent.pb.cc

src/proto/gced_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/gced_master.pb.o src/proto/master.pb.cc

src/proto/gced_galaxy.pb.o:src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced_galaxy.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/gced_galaxy.pb.o src/proto/galaxy.pb.cc

src/proto/gced_initd.pb.o:src/proto/initd.pb.cc \
  src/proto/initd.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/gced_initd.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/gced_initd.pb.o src/proto/initd.pb.cc

src/sdk/galaxy_galaxy.o:src/sdk/galaxy.cc \
  src/sdk/galaxy.h \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h \
  src/rpc/rpc_client.h \
  common/include/mutex.h \
  common/include/timer.h \
  common/include/thread_pool.h \
  common/include/mutex.h \
  common/include/logging.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/sdk/galaxy_galaxy.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/sdk/galaxy_galaxy.o src/sdk/galaxy.cc

src/proto/galaxy_galaxy.pb.o:src/proto/galaxy.pb.cc \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/galaxy_galaxy.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/galaxy_galaxy.pb.o src/proto/galaxy.pb.cc

src/proto/galaxy_master.pb.o:src/proto/master.pb.cc \
  src/proto/master.pb.h \
  src/proto/galaxy.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/proto/galaxy_master.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/proto/galaxy_master.pb.o src/proto/master.pb.cc

src/client/galaxy_galaxy_client.o:src/client/galaxy_client.cc \
  src/sdk/galaxy.h \
  common/include/tprinter.h \
  common/include/string_util.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/client/galaxy_galaxy_client.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/client/galaxy_galaxy_client.o src/client/galaxy_client.cc

src/galaxy_flags.o:src/flags.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msrc/galaxy_flags.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/galaxy_flags.o src/flags.cc

endif #ifeq ($(shell uname -m),x86_64)


