cmake_minimum_required (VERSION 3.31)

include($ENV{ARIEO_BUILDENV_PACKAGE_INSTALL_FOLDER}/cmake/build_environment.cmake)
project(ArieoEngine)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    arieo_engine_project(
        arieo_vulkan_module
        PROJECT_TYPE module

        DEPENDENCIES
            PACKAGES
                Arieo-Interface-RHI
                # vulkan_sdk
                vma
                Arieo-Core
            INTERFACES
                Arieo-Interface-RHI::arieo_rhi_interface
            PRIVATE_LIBS
                Arieo-Core::arieo_core
                vma::vma
                vulkan-1
            PRIVATE_LIB_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_win_x86_64_1.4.328.1/Lib

        SOURCES
            PRIVATE_INCLUDE_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_win_x86_64_1.4.328.1/Include/vulkan
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_win_x86_64_1.4.328.1/Include
            CXX_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    arieo_engine_project(
        arieo_vulkan_module
        PROJECT_TYPE module

        DEPENDENCIES
            PACKAGES
                Arieo-Interface-RHI
                # vulkan_sdk
                vma
                Arieo-Core
            INTERFACES
                Arieo-Interface-RHI::arieo_rhi_interface
            PRIVATE_LIBS
                Arieo-Core::arieo_core
                vma::vma
                vulkan.1
            PRIVATE_LIB_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_macos_x86_64_1.4.328.1/lib

        SOURCES
            PRIVATE_INCLUDE_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_macos_x86_64_1.4.328.1/include/vulkan
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_macos_x86_64_1.4.328.1/include
            CXX_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    arieo_engine_project(
        arieo_vulkan_module
        PROJECT_TYPE module

        DEPENDENCIES
            PACKAGES
                Arieo-Interface-RHI
                vma
                Arieo-Core
            INTERFACES
                Arieo-Interface-RHI::arieo_rhi_interface
            PRIVATE_LIBS
                Arieo-Core::arieo_core
                vulkan
                vma::vma

        SOURCES
            PRIVATE_INCLUDE_FOLDERS
                ${CMAKE_SYSROOT}/usr/include/vulkan
                ${CMAKE_SYSROOT}/usr/include
            CXX_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(ARIEO_HOST_OS MATCHES "ubuntu")
        arieo_engine_project(
            arieo_vulkan_module
            PROJECT_TYPE module

            DEPENDENCIES
                PACKAGES
                    Arieo-Interface-RHI
                    vma
                    Arieo-Core
                INTERFACES
                    Arieo-Interface-RHI::arieo_rhi_interface
                PRIVATE_LIBS
                    Arieo-Core::arieo_core
                    x11
                    vma::vma
                    vulkan

            PRIVATE_LIB_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_linux_x86_64_1.4.328.1/lib

            SOURCES
                PRIVATE_INCLUDE_FOLDERS
                    $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_linux_x86_64_1.4.328.1/include/vulkan
                    $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_linux_x86_64_1.4.328.1/include
                CXX_SOURCES
                    ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
        )
    elseif(ARIEO_HOST_OS MATCHES "raspberry")
        arieo_engine_project(
            arieo_vulkan_module
            PROJECT_TYPE module

            DEPENDENCIES
                PACKAGES
                    Arieo-Interface-RHI
                    vma
                    Arieo-Core
                INTERFACES
                    Arieo-Interface-RHI::arieo_rhi_interface
                PRIVATE_LIBS
                    Arieo-Core::arieo_core
                    vma::vma
                    vulkan
                    x11

            SOURCES
                PRIVATE_INCLUDE_FOLDERS
                    ${CMAKE_SYSROOT}/usr/include/vulkan
                CXX_SOURCES
                    ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
        )
    endif()
endif()
