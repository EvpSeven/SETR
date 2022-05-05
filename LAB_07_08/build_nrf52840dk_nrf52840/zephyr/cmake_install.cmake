# Install script for directory: /opt/ncs/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/opt/gnuarmemb/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/soc/arm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/cddl-gen/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/CANopenNode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/civetweb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/tinycbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/TraceRecorder/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/andrembrandao/Desktop/SETR/LAB_07_08/build_nrf52840dk_nrf52840/zephyr/cmake/reports/cmake_install.cmake")
endif()

