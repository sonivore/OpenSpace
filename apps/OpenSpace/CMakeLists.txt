##########################################################################################
#                                                                                        #
# OpenSpace                                                                              #
#                                                                                        #
# Copyright (c) 2014-2018                                                                #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

include(${GHOUL_BASE_DIR}/support/cmake/copy_shared_libraries.cmake)
include(${OPENSPACE_CMAKE_EXT_DIR}/application_definition.cmake)
include(${OPENSPACE_CMAKE_EXT_DIR}/global_variables.cmake)

#####
# OpenVR
#####
option(OPENSPACE_OPENVR_SUPPORT "Build OpenSpace application with OpenVR support" OFF)

if (OPENSPACE_OPENVR_SUPPORT)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OPENSPACE_EXT_DIR}/sgct/cmake/modules/")

    find_package(OpenVR REQUIRED)

    set(SGCT_OPENVR_DEFINITIONS OPENVR_SUPPORT)

    if (NOT SGCT_OPENVR_INCLUDE_DIRECTORY)
        if (WIN32)
            find_path(SGCT_OPENVR_INCLUDE_DIRECTORY
                NAMES SGCTOpenVR.h
                PATHS
                    $ENV{SGCT_ROOT_DIR}/additional_includes/openvr
                    ${OPENSPACE_EXT_DIR}/sgct/additional_includes/openvr
                    NO_DEFAULT_PATH
                REQUIRED
            )
        else ()
            find_path(SGCT_OPENVR_INCLUDE_DIRECTORY
                NAMES SGCTOpenVR.h
                PATH_SUFFIXES SGCTOpenVR
                PATHS
                    $ENV{SGCT_ROOT_DIR}/additional_includes/openvr
                    ${OPENSPACE_EXT_DIR}/sgct/additional_includes/openvr
                REQUIRED
            )
        endif ()
    else()
        set(
            SGCT_OPENVR_FILES
            ${SGCT_OPENVR_INCLUDE_DIRECTORY}/SGCTOpenVR.h
            ${SGCT_OPENVR_INCLUDE_DIRECTORY}/SGCTOpenVR.cpp
        )
    endif()
endif()

#####
# Spout 
#####
if (SGCT_SPOUT_SUPPORT AND NOT OPENSPACE_MODULE_SPOUT)
    set(SPOUT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ext/spout)
    set(SPOUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/ext/spout/SpoutLibrary.lib)
    set(SPOUT_DEFINITIONS "OPENSPACE_HAS_SPOUT")

    add_external_library_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/ext/spout/SpoutLibrary.dll)
endif ()

set(MACOSX_BUNDLE_ICON_FILE openspace.icns)

create_new_application(OpenSpace
    ${SGCT_OPENVR_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openspace.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/openspace.icns
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/openspace.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
)

target_include_directories(
    OpenSpace PRIVATE
    ${OPENVR_INCLUDE_DIRS}
    ${SGCT_OPENVR_INCLUDE_DIRECTORY}
    ${SPOUT_INCLUDE_DIRS}
)

target_link_libraries(OpenSpace
    libOpenSpace
    ${OPENVR_LIBRARY}
    ${SPOUT_LIBRARY}
)

target_compile_definitions(OpenSpace PRIVATE
    ${SGCT_OPENVR_DEFINITIONS}
    ${SPOUT_DEFINITIONS}
)


# Web Browser and Web gui
# Why not put these in the module's path? Because they do not have access to the
# target as of July 2017, which is needed.
if (OPENSPACE_MODULE_WEBBROWSER AND CEF_ROOT)
    # wanted by CEF
    set(CMAKE_BUILD_TYPE Debug CACHE INTERNAL "CMAKE_BUILD_TYPE")
    
    if (WIN32)
        set(RESOURCE_FILE ${OPENSPACE_APPS_DIR}/OpenSpace/openspace.rc)
    endif ()

    # Add the CEF binary distribution's cmake/ directory to the module path and
    # find CEF to initialize it properly.
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBBROWSER_MODULE_PATH}/cmake")
    include(webbrowser_helpers)

    set_cef_targets("${CEF_ROOT}" OpenSpace)
    run_cef_platform_config("${CEF_ROOT}" "${CEF_TARGET}" "${WEBBROWSER_MODULE_PATH}")
elseif (OPENSPACE_MODULE_WEBBROWSER)
    message(WARNING "Web configured to be included, but no CEF_ROOT was found, please try configuring CMake again.")
endif ()

if (OPENSPACE_MODULE_WEBGUI AND WEBGUI_MODULE_PATH)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBGUI_MODULE_PATH}/cmake")
    include(webgui_helpers)
    build_webgui_source(OpenSpace)
elseif(OPENSPACE_MODULE_WEBGUI)
    message(WARNING "WebGui is configured to be included, but the web source could not be found. Try configuring CMake again.")
endif()
# End Web Browser and Web gui

if (MSVC)
    # This library is used for being able to output the callstack if an exception escapes
    target_link_libraries(OpenSpace Dbghelp.lib)
endif()
