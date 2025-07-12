cmake_minimum_required(VERSION 3.19)

# CRITICAL: Set toolchain before project
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(fluffydiver)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

# VPK Info
set(VITA_APP_NAME "Fluffy Diver")
set(VITA_TITLEID  "FLUF00001")
set(VITA_VERSION  "01.00")

# CRITICAL: Remove softfp flags - use default VitaSDK ABI
# The issue is mixing soft and hard float ABIs
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=softfp")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=softfp")

# Sources
add_executable(fluffydiver
    src/main.c
    jni/so_util.c
)

# Include directories
target_include_directories(fluffydiver PRIVATE jni/)

# CRITICAL: kubridge must be first for so-loader
target_link_libraries(fluffydiver
    kubridge_stub
    vitaGL
    vitashark
    SceShaccCgExt
    SceGxm_stub
    SceDisplay_stub
    SceCtrl_stub
    SceTouch_stub
    SceAudio_stub
    SceIofilemgr_stub
    SceLibKernel_stub
    SceSysmodule_stub
    SceShaccCg_stub
    SceAppMgr_stub
    SceCommonDialog_stub
    SceKernelDmacMgr_stub
    taihen_stub
    mathneon
    pthread
    m
    z
)

# Create VPK
vita_create_self(fluffydiver.self fluffydiver UNSAFE)
vita_create_vpk(fluffydiver.vpk ${VITA_TITLEID} fluffydiver.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_SOURCE_DIR}/lib/libFluffyDiver.so lib/libFluffyDiver.so
)
