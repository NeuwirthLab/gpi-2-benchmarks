cmake_minimum_required(VERSION 3.5)
project(gpi-benchmarks)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

option(WITH_MPI "Compile code such it can run using GPI-2 with MPI support" FALSE)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)

if(WITH_MPI)
	find_package(MPI REQUIRED)
endif()

function(settings target)
	target_link_libraries(
		${target} PRIVATE "GPI2::GPI2" "Threads::Threads" "m" "$<$<BOOL:${WITH_MPI}>:MPI::MPI_C>" "OpenMP::OpenMP_C"
	)
	target_include_directories(
	  ${target} PRIVATE "${CMAKE_SOURCE_DIR}/util"
	)
	target_compile_features(${target} PRIVATE c_std_11)
	target_compile_definitions(${target} PRIVATE "$<$<BOOL:${WITH_MPI}>:USE_MPI>")
endfunction()

set(SRCS "src/one-sided/gbs_read_bw_threads.c" "src/atomic/gbs_atomic_fadd.c" "src/atomic/gbs_atomic_cas.c" "src/collective/gbs_allreduce.c" "src/collective/gbs_barrier.c" "src/gaspi-info/gbs_info.c" "src/notification/gbs_write_notification_ping_pong.c" "src/notification/gbs_notification_ping_pong.c" "src/one-sided/gbs_read_bw.c" "src/one-sided/gbs_read_lat.c" "src/one-sided/gbs_write_bw.c" "src/one-sided/gbs_write_lat.c" "src/one-sided/gbs_write_bibw.c" "src/one-sided-extended/gbs_read_notify_bw.c" "src/one-sided-extended/gbs_read_notify_lat.c" "src/one-sided-extended/gbs_write_notify_bw.c" "src/one-sided-extended/gbs_write_notify_lat.c" "src/one-sided-extended/gbs_write_notify_bibw.c" "src/passive/gbs_passive_bw.c" "src/passive/gbs_passive_lat.c")
set(TRGTS "gbs_read_bw_threads" "gbs_atomic_fadd" "gbs_atomic_cas" "gbs_allreduce" "gbs_barrier" "gbs_info" "gbs_write_notification_ping_pong" "gbs_notification_ping_pong" "gbs_read_bw" "gbs_read_lat" "gbs_write_bw" "gbs_write_lat" "gbs_write_bibw" "gbs_read_notify_bw" "gbs_read_notify_lat" "gbs_write_notify_bw" "gbs_write_notify_lat" "gbs_write_notify_bibw" "gbs_passive_bw" "gbs_passive_lat")
set(UTILS "util/util.c" "util/util_memory.c" "util/stopwatch.c")

foreach( T S IN ZIP_LISTS TRGTS SRCS)
	add_executable(${T} "${S}" "${UTILS}")
	settings(${T})
endforeach()

include(GNUInstallDirs)
install(TARGETS ${TRGTS} DESTINATION ${CMAKE_INSTALL_BINDIR})
