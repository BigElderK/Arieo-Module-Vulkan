cmake_minimum_required (VERSION 3.31)

include($ENV{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)

ARIEO_PACKAGE(
    Arieo-Module-Vulkan
    CATEGORY 02_engine
    DEPENDS
        https://github.com/BigElderK/Arieo-Core.git@main
        https://github.com/BigElderK/Arieo-Interface-RHI.git@main
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    ARIEO_ENGINE_PROJECT(
        arieo_vulkan_module
        PROJECT_TYPE module

        DEPENDENCIES
            ARIEO_PACKAGES
                Arieo-Core
                Arieo-Interface-RHI
            THIRDPARTY_PACKAGES
                # vulkan_sdk
                vma
            PRIVATE_LIBS
                Arieo-Interface-RHI::arieo_rhi_interface
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
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*/*.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    ARIEO_ENGINE_PROJECT(
        arieo_vulkan_module
        PROJECT_TYPE module

        DEPENDENCIES
            ARIEO_PACKAGES
                Arieo-Core
                Arieo-Interface-RHI
            THIRDPARTY_PACKAGES
                # vulkan_sdk
                vma
            PRIVATE_LIBS
                Arieo-Interface-RHI::arieo_rhi_interface
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
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*/*.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    ARIEO_ENGINE_PROJECT(
        arieo_vulkan_module
        PROJECT_TYPE module

        DEPENDENCIES
            ARIEO_PACKAGES
                Arieo-Core
                Arieo-Interface-RHI
            THIRDPARTY_PACKAGES
                vma
            PRIVATE_LIBS
                Arieo-Interface-RHI::arieo_rhi_interface
                Arieo-Core::arieo_core
                vulkan
                vma::vma

        SOURCES
            PRIVATE_INCLUDE_FOLDERS
                ${CMAKE_SYSROOT}/usr/include/vulkan
                ${CMAKE_SYSROOT}/usr/include
            CXX_SOURCES
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*/*.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(ARIEO_HOST_OS MATCHES "ubuntu")
        ARIEO_ENGINE_PROJECT(
            arieo_vulkan_module
            PROJECT_TYPE module

            DEPENDENCIES
                ARIEO_PACKAGES
                    Arieo-Core
                    Arieo-Interface-RHI
                THIRDPARTY_PACKAGES
                    vma
                PRIVATE_LIBS
                    Arieo-Interface-RHI::arieo_rhi_interface
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
                    ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
                    ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
                    ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*/*.cpp
        )
    elseif(ARIEO_HOST_OS MATCHES "raspberry")
        ARIEO_ENGINE_PROJECT(
            arieo_vulkan_module
            PROJECT_TYPE module

            DEPENDENCIES
                ARIEO_PACKAGES
                    Arieo-Core
                    Arieo-Interface-RHI
                THIRDPARTY_PACKAGES
                    vma
                PRIVATE_LIBS
                    Arieo-Interface-RHI::arieo_rhi_interface
                    Arieo-Core::arieo_core
                    vma::vma
                    vulkan
                    x11

            SOURCES
                PRIVATE_INCLUDE_FOLDERS
                    ${CMAKE_SYSROOT}/usr/include/vulkan
                CXX_SOURCES
                    ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
                    ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
                    ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*/*.cpp
        )
    endif()
endif()
