cmake_minimum_required(VERSION 3.19)

# Include VitaSDK BEFORE project declaration
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

# Set project name
project(fluffydiver)

# Include VitaSDK
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

# Set C standard
set(CMAKE_C_STANDARD 99)

# VPK metadata
set(VITA_APP_NAME "Fluffy Diver")
set(VITA_TITLEID  "FLUF00001")
set(VITA_VERSION  "01.00")

# Define executable - NOW WITH SAFE FUNCTION CALLER
add_executable(fluffydiver
    src/main.c
    jni/jni_patch.c
    jni/android_stubs.c
    jni/so_util_simple.c
    jni/falsojni.c
    jni/minimal_function_test.c  # ← Changed from safe_function_caller.c
)

# Include directories
target_include_directories(fluffydiver PRIVATE
    jni/
    src/
)

# Link libraries - order matters for so-loader!
target_link_libraries(fluffydiver
    # so-loader dependencies
    kubridge_stub
    
    # Graphics
    vitaGL
    vitashark
    SceShaccCgExt
    
    # Graphics system libraries
    SceGxm_stub
    SceDisplay_stub
    SceCommonDialog_stub
    
    # Input/System
    SceCtrl_stub
    SceTouch_stub
    SceHid_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAudio_stub
    SceAudioIn_stub
    
    # File system
    SceIofilemgr_stub
    SceLibKernel_stub
    
    # Additional system libraries
    SceSysmodule_stub
    SceShaccCg_stub
    taihen_stub
    
    # Math libraries for VitaGL
    mathneon
    
    # DMA library alternative
    SceKernelDmacMgr_stub
    
    # Standard libraries
    pthread
    m
    z
)

# Create VPK
vita_create_self(fluffydiver.self fluffydiver UNSAFE)
vita_create_vpk(fluffydiver.vpk ${VITA_TITLEID} fluffydiver.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_SOURCE_DIR}/assets assets
    FILE ${CMAKE_SOURCE_DIR}/lib/libFluffyDiver.so lib/libFluffyDiver.so
)
