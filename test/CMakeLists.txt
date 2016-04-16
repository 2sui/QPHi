
# cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# project name
PROJECT(qp_test)

##
# test list.
#

# pcapreader
SET(pcapreader test_pcapreader) # bin name
SET(pcapreader_src pcapreader.c) # src
# tcp_delay_server
SET(tcp_delay test_tcp_delay)
SET(tcp_delay_src tcp_delay.c)
# lock
SET(lock test_lock)
SET(lock_src lock.c)
# events
SET(events test_events)
SET(events_src events.c)
# rbtree
SET(rbtree test_rbtree)
SET(rbtree_src rbtree.c)

# direct file
SET(directFile test_directFile)
SET(directFile_src directfile.c)

# libs output path
SET(BUILD_LIBS  ${PROJECT_SOURCE_DIR}/../build/libs)
# exec output path
SET(BUILD_BIN  ${PROJECT_SOURCE_DIR}/../build/bin)

# deps dir
SET(DEPS_QPHI  ${PROJECT_SOURCE_DIR}/../src/QPCore)

# sources dir
AUX_SOURCE_DIRECTORY(${DEPS_QPHI}  DEP_QPHi)
AUX_SOURCE_DIRECTORY(${DEPS_QPHI}/qp_tiny_stack  DEP_QPHi_QP_TINY_STACK)

# headers dir
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}  ${DEPS_QPHI})

# set libs output path
SET(LIBRARY_OUTPUT_PATH  ${BUILD_LIBS})
# set exec output path
SET(EXECUTABLE_OUTPUT_PATH  ${BUILD_BIN})

# add flag or definitions
ADD_DEFINITIONS(-W -Wall -D_REENTRANT -g -D_FILE_OFFSET_BITS=64 -DQP_DEBUG)

# exec target

# pcapreader
ADD_EXECUTABLE(${pcapreader}  
    ${pcapreader_src} 
    ${DEP_QPHi} 
    ${DEP_QPHi_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(${pcapreader}  pthread dl rt)


# tcp_delay
ADD_EXECUTABLE(${tcp_delay}  
    ${tcp_delay_src} 
    ${DEP_QPHi} 
    ${DEP_QPHi_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(${tcp_delay}  pthread dl rt)


# lock
ADD_EXECUTABLE(${lock}  
    ${lock_src} 
    ${DEP_QPHi} 
    ${DEP_QPHi_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(${lock}  pthread dl rt)

# events
ADD_EXECUTABLE(${events}  
    ${events_src} 
    ${DEP_QPHi} 
    ${DEP_QPHi_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(${events}  pthread dl rt)

# rbtree
ADD_EXECUTABLE(${rbtree}  
    ${rbtree_src} 
    ${DEP_QPHi} 
    ${DEP_QPHi_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(${rbtree}  pthread dl rt)

# directfile
ADD_EXECUTABLE(${directFile}  
    ${directFile_src} 
    ${DEP_QPHi} 
    ${DEP_QPHi_QP_TINY_STACK})
TARGET_LINK_LIBRARIES(${directFile}  pthread dl rt)
