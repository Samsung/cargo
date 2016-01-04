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
# @author Dariusz Michaluk (d.michaluk@samsung.com)
#

MESSAGE(STATUS "Installing configs for the Unit Tests")

## Generate ####################################################################
IF(NOT WITHOUT_SYSTEMD)
CONFIGURE_FILE(systemd/cargo-socket-test.service.in
               ${CMAKE_BINARY_DIR}/systemd/cargo-socket-test.service)
ENDIF(NOT WITHOUT_SYSTEMD)

## Install #####################################################################
INSTALL(FILES        utils/file.txt
        DESTINATION  ${CARGO_TEST_CONFIG_INSTALL_DIR}/utils)

IF(NOT WITHOUT_SYSTEMD)
INSTALL(FILES        systemd/cargo-socket-test.socket
                     ${CMAKE_BINARY_DIR}/systemd/cargo-socket-test.service
        DESTINATION  ${SYSTEMD_UNIT_DIR})
ENDIF(NOT WITHOUT_SYSTEMD)
