# cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# project name
PROJECT(QPCore)

# libs output path
SET(BUILD_LIBS  ${PROJECT_SOURCE_DIR}/../../build/libs)
# exec output path
SET(BUILD_BIN  ${PROJECT_SOURCE_DIR}/../../build/bin)

# sources dir
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}  SRC_ROOT)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/qp_tiny_stack  SRC_QP_TINY_STACK)

# headers dir
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/qp_tiny_stack)

# set libs output path 
SET(LIBRARY_OUTPUT_PATH  ${BUILD_LIBS})
# set exec output path
#SET(EXECUTABLE_OUTPUT_PATH ${BUILD_BIN})

SET(CMAKE_INSTALL_PREFIX /usr)

# add flag or definitions
ADD_DEFINITIONS(-W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DQP_DEBUG -g) 

# lib target
ADD_LIBRARY(output_shared  SHARED  ${SRC_ROOT} ${SRC_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(output_shared  pthread dl rt)
SET_TARGET_PROPERTIES(output_shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
SET_TARGET_PROPERTIES(output_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(output_static  STATIC  ${SRC_ROOT} ${SRC_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(output_static  pthread dl rt)
SET_TARGET_PROPERTIES(output_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
SET_TARGET_PROPERTIES(output_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


INSTALL(FILES ${BUILD_LIBS}/lib${PROJECT_NAME}.a 
        ${BUILD_LIBS}/lib${PROJECT_NAME}.so
        DESTINATION lib) 

INSTALL(FILES ${PROJECT_SOURCE_DIR}/qp_core.h
        ${PROJECT_SOURCE_DIR}/qp_debug.h
        ${PROJECT_SOURCE_DIR}/qp_event.h
        ${PROJECT_SOURCE_DIR}/qp_file.h
        ${PROJECT_SOURCE_DIR}/qp_o_atomic.h
        ${PROJECT_SOURCE_DIR}/qp_o_io.h
        ${PROJECT_SOURCE_DIR}/qp_o_ipc.h
        ${PROJECT_SOURCE_DIR}/qp_o_memory.h
        ${PROJECT_SOURCE_DIR}/qp_o_typedef.h
        ${PROJECT_SOURCE_DIR}/qp_pool.h
        ${PROJECT_SOURCE_DIR}/qp_processes.h
        ${PROJECT_SOURCE_DIR}/qp_socket.h
        ${PROJECT_SOURCE_DIR}/qp_system.h
        ${PROJECT_SOURCE_DIR}/qp_tiny_stack.h
        DESTINATION include)

# exec target
#ADD_EXECUTABLE(${PROJECT_NAME}  ${SRC_LIST} ${SUBSRC_LIST})
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  pthread dl rt)
