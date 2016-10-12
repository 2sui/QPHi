# Cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# Project name
PROJECT(qphi)

# Compiler
#SET (CMAKE_C_COMPILER             "/usr/bin/gcc")
#SET (CMAKE_CXX_COMPILER           "/usr/bin/g++")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" )
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -g3 -Wall -Wno-unused-variable -pthread")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g3 -Wall -Wno-unused-variable -pthread")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
else(CMAKE_BUILD_TYPE STREQUAL "Debug" )
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGSS} -O2 -pthread")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread")
endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )


# Libs output path
SET(BUILD_LIBS  ${PROJECT_SOURCE_DIR}/../../build/lib)
# Exec output path
SET(BUILD_BIN  ${PROJECT_SOURCE_DIR}/../../build/bin)

# Set libs output path 
SET(LIBRARY_OUTPUT_PATH  ${BUILD_LIBS})
# Set exec output path
#SET(EXECUTABLE_OUTPUT_PATH ${BUILD_BIN})

# Install 
SET(CMAKE_INSTALL_PREFIX /usr)

# Set path 
SET(PATH_ROOT  ${PROJECT_SRC_ROOT_DIR})
SET(PATH_CORE  ${PROJECT_SRC_ROOT_DIR}/core)
SET(PATH_EVENT_MODULE  ${PROJECT_SRC_ROOT_DIR}/qp_event_module)
SET(PATH_TINY_STACK ${PROJECT_SRC_ROOT_DIR}/qp_tiny_stack)

# Link libraries
FIND_LIBRARY(LB_SYS NAMES rt dl)
SET(RUNTIME_LIBS "pthread ${LB_SYS}")


# Headers dir
INCLUDE_DIRECTORIES(
    ${PROJECT_SRC_ROOT_DIR} 
    ${PROJECT_SRC_ROOT_DIR}/core
    ${PROJECT_SRC_ROOT_DIR}/qp_event_module
    ${PROJECT_SRC_ROOT_DIR}/qp_tiny_stack)

# Sources dir
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}  SRC_ROOT)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/core  SRC_CORE)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/qp_event_module  SRC_EVENT_MODULE)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/qp_tiny_stack  SRC_TINY_STACK)

# Add flag or definitions
ADD_DEFINITIONS(-W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64)  

# Lib target
ADD_LIBRARY(output_shared  SHARED  
    ${SRC_ROOT} 
    ${SRC_CORE} 
    ${SRC_EVENT_MODULE} 
    ${SRC_TINY_STACK})
TARGET_LINK_LIBRARIES(output_shared  ${RUNTIME_LIBS})
SET_TARGET_PROPERTIES(output_shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
SET_TARGET_PROPERTIES(output_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(output_static  STATIC  
    ${SRC_ROOT} 
    ${SRC_CORE} 
    ${SRC_EVENT_MODULE} 
    ${SRC_TINY_STACK})
TARGET_LINK_LIBRARIES(output_static  ${RUNTIME_LIBS})
SET_TARGET_PROPERTIES(output_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
SET_TARGET_PROPERTIES(output_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# Install 
INSTALL(FILES ${PROJECT_SOURCE_DIR}/core/qp_defines.h
        ${PROJECT_SOURCE_DIR}/core/qp_debug.h
        DESTINATION include/qphi/core)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/qphi.h
        ${PROJECT_SOURCE_DIR}/qp_pool.h
        ${PROJECT_SOURCE_DIR}/qp_socket.h
        ${PROJECT_SOURCE_DIR}/qp_event.h
        ${PROJECT_SOURCE_DIR}/qp_file.h
        ${PROJECT_SOURCE_DIR}/qp_ipc.h
        ${PROJECT_SOURCE_DIR}/qp_process.h
        ${PROJECT_SOURCE_DIR}/qp_system.h
        ${PROJECT_SOURCE_DIR}/qp_tiny_stack.h
        DESTINATION include/qphi)

INSTALL(FILES ${BUILD_LIBS}/lib${PROJECT_NAME}.a 
        ${BUILD_LIBS}/lib${PROJECT_NAME}.so
        DESTINATION lib)

# Exec target
#ADD_EXECUTABLE(${PROJECT_NAME} 
#    ${SRC_ROOT} 
#    ${SRC_CORE} 
#    ${SRC_EVENT_MODULE} 
#    ${SRC_QP_TINY_STACK})
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${RUNTIME_LIBS})

message("=================== info =======================
CMake Build Type: ${CMAKE_BUILD_TYPE}
Flags: 
  c: ${CMAKE_C_FLAGS} 
  c debug: ${CMAKE_C_FLAGS_DEBUG}
  c++: ${CMAKE_CXX_FLAGS}
  c++ debug: ${CMAKE_CXX_FLAGS_DEBUG}
Linked Libraries: ${RUNTIME_LIBS}
CMake Build Library output path: ${BUILD_LIBS}
CMake Build exec output path: ${BUILD_BIN}
================================================")
