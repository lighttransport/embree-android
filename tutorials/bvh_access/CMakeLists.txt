## Copyright 2009-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

IF (MINGW)
  ADD_EXECUTABLE(bvh_access ../../kernels/embree.mrc bvh_access.cpp)
ELSE()
  ADD_EXECUTABLE(bvh_access ../../kernels/embree.rc bvh_access.cpp)
ENDIF()
TARGET_LINK_LIBRARIES(bvh_access embree math sys tasking tutorial ${GLFW_LIBRARY})
SET_PROPERTY(TARGET bvh_access PROPERTY FOLDER tutorials/single)
SET_PROPERTY(TARGET bvh_access APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
INSTALL(TARGETS bvh_access DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT examples)
SIGN_TARGET(bvh_access)

IF (BUILD_TESTING AND EMBREE_TESTING_INTENSITY GREATER 0)
  ADD_TEST(NAME bvh_access COMMAND bvh_access)
ENDIF()
