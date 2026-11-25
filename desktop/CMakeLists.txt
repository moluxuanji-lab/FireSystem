cmake_minimum_required(VERSION 3.16)

project(FireSystem VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS 
    Quick
    QuickControls2
    Widgets
    PrintSupport
)

qt_standard_project_setup(REQUIRES 6.8)

set(QT_VERSION ${Qt6_VERSION})
set(FLUENTUI_BUILD_STATIC_LIB ON CACHE BOOL "Build FluentUI as static library" FORCE)
set(FLUENTUI_BUILD_EXAMPLES OFF CACHE BOOL "Disable FluentUI examples" FORCE)
set(QT_NO_STANDARD_PROJECT_SETUP ON)
add_subdirectory(third_party/FluentUI/src EXCLUDE_FROM_ALL)
unset(QT_NO_STANDARD_PROJECT_SETUP)
if(TARGET fluentui)
    if(MSVC)
        target_compile_options(fluentui PRIVATE /wd4996)
    else()
        target_compile_options(fluentui PRIVATE -Wno-deprecated-declarations)
    endif()
endif()

include(GNUInstallDirs)
add_subdirectory(src)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Desktop Application")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/packages")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;IFW")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(_pkg_platform "windows-arm64")
    else()
        set(_pkg_platform "windows-x64")
    endif()
    set(_ifw_search_globs
        "C:/Qt/Tools/QtInstallerFramework/*"
        "$ENV{HOMEDRIVE}/Qt/Tools/QtInstallerFramework/*"
    )
    set(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDir@/${PROJECT_NAME}")
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/${PROJECT_NAME}.exe")
    set(_ifw_install_bindir ".")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP;IFW")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(_pkg_platform "macos-arm64")
    else()
        set(_pkg_platform "macos-x64")
    endif()
    set(_ifw_search_globs
        "$ENV{HOME}/Qt/Tools/QtInstallerFramework/*"
        "/opt/Qt/Tools/QtInstallerFramework/*"
    )
    set(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDir@")
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}")
    set(_ifw_install_bindir ".")
else()
    set(CPACK_GENERATOR "TGZ;IFW")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(_pkg_platform "linux-arm64")
    else()
        set(_pkg_platform "linux-x64")
    endif()
    set(_ifw_search_globs
        "$ENV{HOME}/Qt/Tools/QtInstallerFramework/*"
        "/opt/Qt/Tools/QtInstallerFramework/*"
    )
    set(CPACK_IFW_TARGET_DIRECTORY "@HomeDir@/${PROJECT_NAME}")
    set(CPACK_IFW_ADMIN_TARGET_DIRECTORY "@ApplicationsDir@/${PROJECT_NAME}")
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    set(_ifw_install_bindir "${CMAKE_INSTALL_BINDIR}")
endif()

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/CPackOptions.cmake.in"
    "${CMAKE_BINARY_DIR}/CPackOptions.cmake"
    @ONLY
)
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackOptions.cmake")

if(MSVC)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT App)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
    include(InstallRequiredSystemLibraries)
endif()

# Auto-detect Qt Installer Framework (CMake's built-in search stops at IFW 4.5)
if(NOT CPACK_IFW_ROOT AND NOT DEFINED ENV{CPACK_IFW_ROOT} AND NOT DEFINED ENV{QTIFWDIR})
    file(GLOB _ifw_candidates LIST_DIRECTORIES true ${_ifw_search_globs})
    if(_ifw_candidates)
        list(SORT _ifw_candidates COMPARE NATURAL ORDER DESCENDING)
        list(GET _ifw_candidates 0 CPACK_IFW_ROOT)
    else()
        find_program(_ifw_binarycreator NAMES binarycreator)
        if(_ifw_binarycreator)
            get_filename_component(_ifw_bin_dir "${_ifw_binarycreator}" DIRECTORY)
            get_filename_component(CPACK_IFW_ROOT "${_ifw_bin_dir}" DIRECTORY)
        endif()
    endif()
endif()

set(CPACK_IFW_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_IFW_PACKAGE_TITLE "${PROJECT_NAME} Installer")
set(CPACK_IFW_PACKAGE_PUBLISHER "${PROJECT_NAME}")
set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY "${PROJECT_NAME}")
set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS ON)
set(CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH ON)
set(CPACK_IFW_PACKAGE_RUN_PROGRAM_DESCRIPTION "Launch ${PROJECT_NAME}")

file(READ "${CMAKE_SOURCE_DIR}/installer/installscript.qs.in" _ifw_script_content)
string(REPLACE "__PROJECT_NAME__" "${PROJECT_NAME}" _ifw_script_content "${_ifw_script_content}")
string(REPLACE "__INSTALL_BINDIR__" "${_ifw_install_bindir}" _ifw_script_content "${_ifw_script_content}")
file(WRITE "${CMAKE_BINARY_DIR}/installer/installscript.qs" "${_ifw_script_content}")

include(CPack)
include(CPackIFW)

cpack_add_component(App
    DISPLAY_NAME "${PROJECT_NAME}"
    DESCRIPTION "${PROJECT_NAME} Desktop Application"
    REQUIRED
)
cpack_ifw_configure_component(App
    FORCED_INSTALLATION
    DISPLAY_NAME "${PROJECT_NAME}" zh_CN "${PROJECT_NAME}"
    DESCRIPTION "${PROJECT_NAME} Desktop Application" zh_CN "${PROJECT_NAME} 桌面客户端"
    SCRIPT "${CMAKE_BINARY_DIR}/installer/installscript.qs"
)
