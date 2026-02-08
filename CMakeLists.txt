cmake_minimum_required (VERSION 3.31)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    arieo_engine_project(
        arieo_vulkan_module
        ALIAS arieo::modules::rhi::vulkan
        PROJECT_TYPE module
        SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
        INTERFACES
            arieo::interface::rhi
        PACKAGES
            # vulkan_sdk
            vma
        PRIVATE_INCLUDE_FOLDERS
            $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_win_x86_64_1.4.328.1/Include/vulkan
            $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_win_x86_64_1.4.328.1/Include
        PRIVATE_LIB_FOLDERS
            $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_win_x86_64_1.4.328.1/Lib
        LIBS
            arieo::base
            arieo::core
            vma::vma
            vulkan-1
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    arieo_engine_project(
        arieo_vulkan_module
        ALIAS arieo::modules::rhi::vulkan
        PROJECT_TYPE module
        SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
        INTERFACES
            arieo::interface::rhi
        PACKAGES
            # vulkan_sdk
            vma
        PRIVATE_INCLUDE_FOLDERS
            $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_macos_x86_64_1.4.328.1/include/vulkan
            $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_macos_x86_64_1.4.328.1/include
        PRIVATE_LIB_FOLDERS
            $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_macos_x86_64_1.4.328.1/lib
        LIBS
            arieo::base
            arieo::core
            vma::vma
            vulkan.1
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    arieo_engine_project(
        arieo_vulkan_module
        ALIAS arieo::modules::rhi::vulkan
        PROJECT_TYPE module
        SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
        INTERFACES
            arieo::interface::rhi
        PACKAGES
            vma
        PRIVATE_INCLUDE_FOLDERS
            ${CMAKE_SYSROOT}/usr/include/vulkan
            ${CMAKE_SYSROOT}/usr/include
        PRIVATE_LIB_FOLDERS
        LIBS
            arieo::base
            arieo::core
            vulkan
            vma::vma
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(ARIEO_HOST_OS MATCHES "ubuntu")
        arieo_engine_project(
            arieo_vulkan_module
            ALIAS arieo::modules::rhi::vulkan
            PROJECT_TYPE module
            SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
            INTERFACES
                arieo::interface::rhi
            PACKAGES
                vma
            PRIVATE_INCLUDE_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_linux_x86_64_1.4.328.1/include/vulkan
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_linux_x86_64_1.4.328.1/include
            PRIVATE_LIB_FOLDERS
                $ENV{DEVCRAFT_WORKSPACE}/vulkan/vulkansdk_linux_x86_64_1.4.328.1/lib
            LIBS
                arieo::base
                arieo::core
                x11
                vma::vma
                vulkan
        )
    elseif(ARIEO_HOST_OS MATCHES "raspberry")
        arieo_engine_project(
            arieo_vulkan_module
            ALIAS arieo::modules::rhi::vulkan
            PROJECT_TYPE module
            SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*/*.cpp
            INTERFACES
                arieo::interface::rhi
            PACKAGES
                vma
            PRIVATE_INCLUDE_FOLDERS
                ${CMAKE_SYSROOT}/usr/include/vulkan
            PRIVATE_LIB_FOLDERS
            LIBS
                arieo::base
                arieo::core
                arieo::interface::rhi
                vma::vma
                vulkan
                x11
        )
    endif()
endif()
