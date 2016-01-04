# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
# @file   CMakeLists.txt
# @author Pawel Kubik (p.kubik@samsung.com)
#

PROJECT(cargo-utils)

MESSAGE(STATUS "")
MESSAGE(STATUS "Generating makefile for the libcargo-utils...")
FILE(GLOB_RECURSE SRCS *.cpp *.hpp)

## Setup target ################################################################
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})

FIND_PACKAGE (Boost REQUIRED COMPONENTS system filesystem)
PKG_CHECK_MODULES(CARGO_UTILS_DEPS REQUIRED glib-2.0)

INCLUDE_DIRECTORIES(${COMMON_FOLDER} ${LIBS_FOLDER})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS} ${CARGO_UTILS_DEPS_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES} ${CARGO_UTILS_DEPS_LIBRARIES})

## Install #####################################################################
INSTALL(TARGETS     ${PROJECT_NAME}
        DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT   DevelopmentLibraries)

INSTALL(DIRECTORY . DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.hpp"
                       PATTERN "CMakeFiles" EXCLUDE)
