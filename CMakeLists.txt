cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(fluffydiver)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Fluffy Diver")
set(VITA_TITLEID  "FLUF00001")
set(VITA_VERSION  "01.00")

# Don't override the default ABI - let VitaSDK use its default
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-q -D_GNU_SOURCE -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-q -D_GNU_SOURCE -Wall -O3")

add_executable(fluffydiver
  src/main.c
  src/so_util.c
  src/jni_patch.c
  src/default_dynlib.c
  src/io_patch.c
  src/pthread_patch.c
  src/sys_utils.c
)

target_include_directories(fluffydiver PRIVATE include)

# Fixed linking order - libraries that depend on others must come first
target_link_libraries(fluffydiver
  -Wl,--whole-archive pthread -Wl,--no-whole-archive
  vitaGL
  vitashark
  mathneon
  m
  SceShaccCgExt
  taihen_stub
  kubridge_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceCommonDialog_stub
  SceCtrl_stub
  SceDisplay_stub
  SceKernelDmacMgr_stub
  SceFios2_stub
  SceGxm_stub
  SceShaccCg_stub
  SceSysmodule_stub
  ScePower_stub
  SceTouch_stub
  SceVshBridge_stub
  SceMotion_stub
  z
  stdc++
)

vita_create_self(fluffydiver.self fluffydiver UNSAFE)
vita_create_vpk(fluffydiver.vpk ${VITA_TITLEID} fluffydiver.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libFluffyDiver.so lib/libFluffyDiver.so
)
