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
set(VITA_VERSION  "01.02")  # Incremented for restored version

# Don't override the default ABI - let VitaSDK use its default
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-q -D_GNU_SOURCE -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-q -D_GNU_SOURCE -Wall -O3")

# Restored source files - back to working approach
add_executable(fluffydiver
  src/main.c
  src/so_util.c
  src/jni_patch.c
  src/android_patch.c
  src/android_api.c
  src/default_dynlib.c
  src/io_patch.c
  src/pthread_patch.c
  src/sys_utils.c
)

target_include_directories(fluffydiver PRIVATE include)

# GTA SA Vita exact linking configuration + Thread management for hang detection
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
  SceKernelThreadMgr_stub  # Added for hang detection thread
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

# GTA SA Vita VPK structure - ONLY binary files, NO assets
vita_create_self(fluffydiver.self fluffydiver UNSAFE)
vita_create_vpk(fluffydiver.vpk ${VITA_TITLEID} fluffydiver.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  # ONLY include the SO library - NO ASSETS
  FILE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libFluffyDiver.so lib/libFluffyDiver.so
)

# Add custom target to show restoration info
add_custom_target(restore_info ALL
  COMMAND ${CMAKE_COMMAND} -E echo "=================================="
  COMMAND ${CMAKE_COMMAND} -E echo "Fluffy Diver PS Vita - RESTORED"
  COMMAND ${CMAKE_COMMAND} -E echo "Back to original working approach"
  COMMAND ${CMAKE_COMMAND} -E echo "=================================="
  COMMAND ${CMAKE_COMMAND} -E echo "Changes from enhanced version:"
  COMMAND ${CMAKE_COMMAND} -E echo "- Removed crash protection (was causing crash)"
  COMMAND ${CMAKE_COMMAND} -E echo "- Restored simple symbol analysis"
  COMMAND ${CMAKE_COMMAND} -E echo "- Added hang detection thread"
  COMMAND ${CMAKE_COMMAND} -E echo "- Direct function calls (like original)"
  COMMAND ${CMAKE_COMMAND} -E echo "- GTA SA Vita timeout methodology"
  COMMAND ${CMAKE_COMMAND} -E echo "=================================="
  COMMAND ${CMAKE_COMMAND} -E echo "Expected: Function call will hang = SUCCESS"
  COMMAND ${CMAKE_COMMAND} -E echo "=================================="
  VERBATIM
)
