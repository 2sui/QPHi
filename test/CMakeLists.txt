
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

# libevent file
SET(libevent test_libevent)
SET(libevent_src libevent.c)

# libs output path
SET(BUILD_LIBS  ${PROJECT_SOURCE_DIR}/../build/libs)
# exec output path
SET(BUILD_BIN  ${PROJECT_SOURCE_DIR}/../build/bin)

# headers dir
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

# set libs output path
SET(LIBRARY_OUTPUT_PATH  ${BUILD_LIBS})
# set exec output path
SET(EXECUTABLE_OUTPUT_PATH  ${BUILD_BIN})

# add flag or definitions
ADD_DEFINITIONS(-W -Wall -D_REENTRANT -g -D_FILE_OFFSET_BITS=64 -DQP_DEBUG)

# exec target

# pcapreader
#ADD_EXECUTABLE(${pcapreader}
#    ${pcapreader_src} )
#TARGET_LINK_LIBRARIES(${pcapreader}  pthread dl rt qphi)

# tcp_delay
#ADD_EXECUTABLE(${tcp_delay}
#    ${tcp_delay_src} )
#TARGET_LINK_LIBRARIES(${tcp_delay}  pthread dl rt qphi)

# lock
#ADD_EXECUTABLE(${lock}
#    ${lock_src} )
#TARGET_LINK_LIBRARIES(${lock}  pthread dl rt qphi)

# events
ADD_EXECUTABLE(${events}
    ${events_src})
TARGET_LINK_LIBRARIES(${events}  pthread dl rt qphi)

# rbtree
#ADD_EXECUTABLE(${rbtree}
#    ${rbtree_src} )
#    ${DEP_QPHi} 
#    ${DEP_QPHi_QP_TINY_STACK})
#TARGET_LINK_LIBRARIES(${rbtree}  pthread dl rt)

# directfile
#ADD_EXECUTABLE(${directFile}
#    ${directFile_src} )
#TARGET_LINK_LIBRARIES(${directFile}  pthread dl rt qphi)

#ADD_EXECUTABLE(${libevent}
#    ${libevent_src} )
 #   ${DEP_QPHi} 
 #   ${DEP_QPHi_QP_TINY_STACK})
#TARGET_LINK_LIBRARIES(${libevent}  pthread dl rt event qphi)