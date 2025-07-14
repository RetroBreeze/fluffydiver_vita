cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(FLUFFYDIVER)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Fluffy Diver")
set(VITA_TITLEID  "FLUF00001")
set(VITA_VERSION  "01.04")  # Updated version with complete implementation
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")

# GTA SA Vita exact compiler flags - CRITICAL for so-loader compatibility
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-q,--wrap,memcpy,--wrap,memmove,--wrap,memset -D_GNU_SOURCE -Wall -O3 -mfloat-abi=softfp")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")

# Complete source file list - ALL GTA SA Vita components included
add_executable(FLUFFYDIVER
  # Main application
  src/main.c

  # Core so-loader components (GTA SA Vita)
  src/so_util.c
  src/jni_patch.c
  src/default_dynlib.c

  # CRITICAL: Missing components from successful ports
  src/config.c
  src/dialog.c
  src/fios.c

  # Android compatibility layer
  src/android_patch.c
  src/android_api.c

  # System utilities
  src/io_patch.c
  src/pthread_patch.c
  src/sys_utils.c
)

# Include directories
target_include_directories(FLUFFYDIVER PRIVATE include)

# GTA SA Vita exact linking configuration - CRITICAL for compatibility
target_link_libraries(FLUFFYDIVER
  # Core pthread support (GTA SA Vita exact)
  -Wl,--whole-archive pthread -Wl,--no-whole-archive

  # Graphics and shader support
  vitaGL
  vitashark
  mathneon
  SceShaccCgExt

  # Core system libraries
  m
  stdc++

  # Kernel and system bridges (CRITICAL)
  taihen_stub
  kubridge_stub

  # System services (GTA SA Vita exact)
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceCommonDialog_stub  # CRITICAL for dialog.c
  SceCtrl_stub
  SceDisplay_stub
  SceKernelDmacMgr_stub
  SceKernelThreadMgr_stub
  SceFios2_stub  # CRITICAL for fios.c
  SceGxm_stub
  SceShaccCg_stub
  SceSysmodule_stub
  ScePower_stub
  SceTouch_stub
  SceVshBridge_stub
  SceMotion_stub

  # Compression and utility
  z
)

# GTA SA Vita VPK structure
vita_create_self(FLUFFYDIVER.self FLUFFYDIVER UNSAFE)
vita_create_vpk(FLUFFYDIVER.vpk ${VITA_TITLEID} FLUFFYDIVER.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}

  # Include game library
  FILE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libFluffyDiver.so lib/libFluffyDiver.so

  # Include LiveArea assets (if available)
  # FILE ${CMAKE_CURRENT_SOURCE_DIR}/livearea/bg.png sce_sys/livearea/contents/bg.png
  # FILE ${CMAKE_CURRENT_SOURCE_DIR}/livearea/startup.png sce_sys/livearea/contents/startup.png
  # FILE ${CMAKE_CURRENT_SOURCE_DIR}/livearea/template.xml sce_sys/livearea/contents/template.xml
)

# Build information target
add_custom_target(build_info ALL
  COMMAND ${CMAKE_COMMAND} -E echo "=============================================="
  COMMAND ${CMAKE_COMMAND} -E echo "Fluffy Diver PS Vita Port - COMPLETE BUILD"
  COMMAND ${CMAKE_COMMAND} -E echo "Based on comprehensive analysis of successful ports:"
  COMMAND ${CMAKE_COMMAND} -E echo "- Modern Combat 3 (v-atamanenko)"
  COMMAND ${CMAKE_COMMAND} -E echo "- Mass Effect Infiltrator (v-atamanenko)"
  COMMAND ${CMAKE_COMMAND} -E echo "- Galaxy on Fire 2 (v-atamanenko)"
  COMMAND ${CMAKE_COMMAND} -E echo "- The Conduit HD (TheOfficialFloW)"
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "NEW COMPONENTS ADDED:"
  COMMAND ${CMAKE_COMMAND} -E echo "✓ config.c - Configuration system"
  COMMAND ${CMAKE_COMMAND} -E echo "✓ dialog.c - Error dialog system"
  COMMAND ${CMAKE_COMMAND} -E echo "✓ fios.c - File I/O management"
  COMMAND ${CMAKE_COMMAND} -E echo "✓ Enhanced Android environment"
  COMMAND ${CMAKE_COMMAND} -E echo "✓ Complete initialization sequence"
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "EXPECTED OUTCOME: Working game launch"
  COMMAND ${CMAKE_COMMAND} -E echo "=============================================="
  VERBATIM
)

# Development convenience targets (from successful ports)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vitacompanion.py")
  add_custom_target(send
    COMMAND ${CMAKE_COMMAND} --build . --target FLUFFYDIVER
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/vitacompanion.py eboot.bin FLUFFYDIVER.self
    DEPENDS FLUFFYDIVER.self
    COMMENT "Building and uploading to Vita via vitacompanion"
  )
endif()

# Core dump analysis target
add_custom_target(dump
  COMMAND ${CMAKE_COMMAND} -E echo "Fetching core dump..."
  COMMAND python3 -c "
import subprocess
import sys
try:
    subprocess.run(['vita-parse-core', 'ux0:data/core'], check=True)
except FileNotFoundError:
    print('vita-parse-core not found - install vita-parse-core for crash analysis')
except subprocess.CalledProcessError:
    print('No core dump found or parsing failed')
"
  COMMENT "Analyzing crash dump"
)

# Print configuration
message(STATUS "Fluffy Diver PS Vita Port Configuration:")
message(STATUS "  App Name: ${VITA_APP_NAME}")
message(STATUS "  Title ID: ${VITA_TITLEID}")
message(STATUS "  Version: ${VITA_VERSION}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  Components: ALL GTA SA Vita components included")
message(STATUS "  Expected: Complete Android environment emulation")
