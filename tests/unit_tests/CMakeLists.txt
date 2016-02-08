# Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
# @author Jan Olszak (j.olszak@samsung.com)
#

MESSAGE(STATUS "")
MESSAGE(STATUS "Generating makefile for the Unit Tests...")

FILE(GLOB_RECURSE project_SRCS *.cpp *.hpp)
FILE(GLOB         socket_test_SRCS ${SOCKET_TEST_FOLDER}/*.cpp ${SOCKET_TEST_FOLDER}/*.hpp)

# We must compile socket-test separately, exclude it from unit-test build
LIST(REMOVE_ITEM project_SRCS ${socket_test_SRCS})

## Setup target ################################################################
SET(UT_SERVER_CODENAME "${PROJECT_NAME}-unit-tests")
ADD_EXECUTABLE(${UT_SERVER_CODENAME} ${project_SRCS})

IF(NOT WITHOUT_SYSTEMD)
SET(SOCKET_TEST_CODENAME "${PROJECT_NAME}-socket-test")

## A stub mini-service to test socket functionality
ADD_EXECUTABLE(${SOCKET_TEST_CODENAME} ${socket_test_SRCS})
ENDIF(NOT WITHOUT_SYSTEMD)

## Link libraries ##############################################################
FIND_PACKAGE (Boost REQUIRED COMPONENTS unit_test_framework system filesystem)

INCLUDE_DIRECTORIES(${COMMON_FOLDER} ${LIBS_FOLDER} ${UNIT_TESTS_FOLDER} ${SOCKET_TEST_FOLDER})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS} ${JSON_C_INCLUDE_DIRS} ${CARGO_IPC_DEPS_INCLUDE_DIRS})

SET_TARGET_PROPERTIES(${UT_SERVER_CODENAME} PROPERTIES
    COMPILE_FLAGS "-pthread"
    LINK_FLAGS "-pthread"
)

TARGET_LINK_LIBRARIES(${UT_SERVER_CODENAME} ${Boost_LIBRARIES}
                      Logger ${JSON_C_LIBRARIES} cargo-sqlite cargo-fd cargo-ipc cargo-validator)

IF(NOT WITHOUT_SYSTEMD)
SET_TARGET_PROPERTIES(${SOCKET_TEST_CODENAME} PROPERTIES
    COMPILE_FLAGS "-pthread"
    LINK_FLAGS "-pthread"
)

TARGET_LINK_LIBRARIES(${SOCKET_TEST_CODENAME} ${Boost_LIBRARIES} Logger cargo-ipc)
ENDIF(NOT WITHOUT_SYSTEMD)

## Subdirectories ##############################################################
SET(CARGO_TEST_CONFIG_INSTALL_DIR ${CARGO_CONFIG_INSTALL_DIR}/tests)
ADD_DEFINITIONS(-DCARGO_TEST_CONFIG_INSTALL_DIR="${CARGO_TEST_CONFIG_INSTALL_DIR}")

ADD_SUBDIRECTORY(configs)

## Install #####################################################################
INSTALL(TARGETS ${UT_SERVER_CODENAME} DESTINATION bin)

IF(NOT WITHOUT_SYSTEMD)
INSTALL(TARGETS ${SOCKET_TEST_CODENAME} DESTINATION bin)
ENDIF(NOT WITHOUT_SYSTEMD)
