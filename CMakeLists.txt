cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PROJECT_NAME)
    set(PROJECT_NAME "RandomNamePicker" CACHE STRING "Project name")
endif()

if(NOT DEFINED PROJECT_VERSION)
    # Auto-detect version from git: tag vX.Y.Z → X.Y.Z, else commit msg X.Y.Z, else git log, else 0.0.0
    set(PROJECT_VERSION "0.0.0")
    find_package(Git QUIET)
    if(Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*" --abbrev=0
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE GIT_TAG_RESULT
        )
        if(GIT_TAG_RESULT EQUAL 0 AND GIT_TAG MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)$")
            set(PROJECT_VERSION "${CMAKE_MATCH_1}")
        else()
            execute_process(
                COMMAND ${GIT_EXECUTABLE} log -1 --format=%s
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE HEAD_MSG
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(HEAD_MSG MATCHES "^([0-9]+\\.[0-9]+\\.[0-9]+)$")
                set(PROJECT_VERSION "${CMAKE_MATCH_1}")
            else()
                execute_process(
                    COMMAND ${GIT_EXECUTABLE} log --first-parent --format=%s -100
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE COMMIT_MSGS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
                )
                string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" PROJECT_VERSION "${COMMIT_MSGS}")
                if(NOT PROJECT_VERSION)
                    set(PROJECT_VERSION "0.0.0")
                endif()
            endif()
        endif()
    endif()
    set(PROJECT_VERSION "${PROJECT_VERSION}" CACHE STRING "Project version")
endif()

project(
    ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Random Name Picker - Classroom roll-call application"
    LANGUAGES CXX
)

cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0077 NEW)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation prefix")
set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "Binary installation directory" FORCE)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" OFF)

# Source files for console application
set(CONSOLE_SOURCES
    src/main.cpp
    src/utils/platform.cpp
    src/core/name_list.cpp
    src/core/config_manager.cpp
    src/core/randomizer.cpp
    src/ui/console.cpp
    src/ui/tui.cpp
    src/ui/setup_tui.cpp
    src/i18n/localizer.cpp
)

# GUI sources
file(GLOB_RECURSE GUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

list(LENGTH CONSOLE_SOURCES CONSOLE_COUNT)
list(LENGTH GUI_SOURCES GUI_COUNT)
list(LENGTH HEADERS HEADERS_COUNT)

# Console application
add_executable(RandomNamePicker ${CONSOLE_SOURCES}
    src/utils/display_util.h)
set_target_properties(RandomNamePicker PROPERTIES
    OUTPUT_NAME "RandomNamePicker"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_include_directories(RandomNamePicker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(RandomNamePicker PRIVATE APP_VERSION="${PROJECT_VERSION}")

# --- i18n: auto-generate .qm from .ts (.qm files are NOT tracked in git) ---
find_program(LRELEASE_EXECUTABLE NAMES lrelease lrelease-qt6)
set(TRANSLATION_TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/trans/RandomNamePicker_en_US.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/trans/RandomNamePicker_zh_CN.ts
)
set(GENERATED_QM_FILES)
if(LRELEASE_EXECUTABLE)
    foreach(TS_FILE IN LISTS TRANSLATION_TS_FILES)
        get_filename_component(TS_NAME "${TS_FILE}" NAME_WE)
        set(QM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/trans/${TS_NAME}.qm")
        add_custom_command(
            OUTPUT "${QM_FILE}"
            COMMAND "${LRELEASE_EXECUTABLE}" "${TS_FILE}" -qm "${QM_FILE}"
            DEPENDS "${TS_FILE}"
            COMMENT "lrelease: generating ${TS_NAME}.qm"
        )
        list(APPEND GENERATED_QM_FILES "${QM_FILE}")
    endforeach()
else()
    message(WARNING "lrelease not found - translation .qm files will not be generated automatically")
endif()

# GUI application (requires Qt6)
find_package(Qt6 QUIET COMPONENTS Core Widgets Gui)
if(Qt6_FOUND)
    add_executable(RandomNamePickerGUI
        ${GUI_SOURCES}
        ${GENERATED_QM_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/i18n/localizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/config_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/name_list.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/platform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/randomizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc
    )
    set_target_properties(RandomNamePickerGUI PROPERTIES
        OUTPUT_NAME "RandomNamePickerGUI"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

    target_include_directories(RandomNamePickerGUI
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_link_libraries(RandomNamePickerGUI
        PRIVATE
            Qt6::Core
            Qt6::Widgets
            Qt6::Gui
    )

    add_custom_command(TARGET RandomNamePickerGUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ui_files/MainWindow.ui
        ${CMAKE_BINARY_DIR}/bin/
        COMMENT "Copying UI file to build directory"
    )

    add_custom_command(TARGET RandomNamePickerGUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/trans/RandomNamePicker_zh_CN.qm
        ${CMAKE_BINARY_DIR}/bin/
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/trans/RandomNamePicker_en_US.qm
        ${CMAKE_BINARY_DIR}/bin/
        COMMENT "Copying translation files to build directory"
    )

    if(WIN32)
        set_target_properties(RandomNamePickerGUI PROPERTIES
            WIN32_EXECUTABLE TRUE
            SUFFIX ".exe"
        )
    elseif(APPLE)
        set_target_properties(RandomNamePickerGUI PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.${PROJECT_NAME}.${PROJECT_NAME}GUI"
            MACOSX_BUNDLE_BUNDLE_NAME "RandomNamePicker"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        )
    endif()
endif()

install(TARGETS RandomNamePicker
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(Qt6_FOUND)
    install(TARGETS RandomNamePickerGUI
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/data")
    install(DIRECTORY data/ DESTINATION ${CMAKE_INSTALL_BINDIR}/data)
endif()

add_custom_target(create_data_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/data
    COMMENT "Creating data directory"
)

add_custom_command(TARGET RandomNamePicker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/trans ${CMAKE_BINARY_DIR}/bin/trans
    COMMENT "Copying translation files"
)

if(BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*_test.cpp"
    )
    if(TEST_SOURCES)
        add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
        target_link_libraries(${PROJECT_NAME}_tests PRIVATE RandomNamePicker)
        add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
    endif()
endif()

message(STATUS "==========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(Qt6_FOUND)
    message(STATUS "Qt version: 6")
endif()
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Console sources: ${CONSOLE_COUNT}")
message(STATUS "GUI sources: ${GUI_COUNT}")
message(STATUS "Header files: ${HEADERS_COUNT}")
message(STATUS "==========================================")

include(InstallRequiredSystemLibraries)
include(CPack)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Jianyin Li")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Random Name Picker")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP;DragNDrop")
else()
    set(CPACK_GENERATOR "ZIP;TGZ")
endif()

set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"
    "/\\\\.github/"
    "/build/"
    "/\\\\.vscode/"
    "\\\\.swp$"
    "\\\\.bak$"
    "\\\\.tmp$"
)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
