/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QP_EVENT_H
#define QP_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"

    
# if defined(QP_OS_LINUX)
#  ifndef  EPOLLEXCLUSIVE
#   define  EPOLLEXCLUSIVE      0
#  endif
#  define  QP_EPOLL_ET           EPOLLET
#  define  QP_EPOLL_ONESHOT      EPOLLONESHOT
#  define  QP_EPOLL_IN           EPOLLIN               
#  define  QP_EPOLL_OUT          EPOLLOUT
#  define  QP_EPOLL_ERR          EPOLLERR              
#  define  QP_EPOLL_HUP          EPOLLHUP             
#  define  QP_EPOLL_RDHUP        EPOLLRDHUP 
#  define  QP_EPOLL_EXCLUSIVE    EPOLLEXCLUSIVE
# else
#  if defined(QP_OS_BSD)
#   define  QP_EPOLL_EXCLUSIVE    0
#   define  QP_EPOLL_ET           0
#   define  QP_EPOLL_ONESHOT      EV_ONESHOT
#   define  QP_EPOLL_IN           EVFILT_READ               
#   define  QP_EPOLL_OUT          EVFILT_WRITE
#   define  QP_EPOLL_ERR          EV_ERROR              
#   define  QP_EPOLL_HUP          0             
#   define  QP_EPOLL_RDHUP        0 
#  else 
   // other system
#  endif
# endif

    
# define  QP_EVENT_READCACHE_SIZE    4096    // common buffer size
    
    
/* event fd stat */
typedef enum {
    QP_EVENT_IDL = 0,
    QP_EVENT_NEW = 1<<0,     /* event is new */
    QP_EVENT_PROCESS = 1<<1, /* event is running */
    QP_EVENT_CLOSE = 1<<2    /* event is closed */
} qp_event_stat_t;


/* buf type */
typedef enum  {
    QP_EVENT_BLOCK_OPT = 0,  /* do read/write with block buf */
    QP_EVENT_VECT_OPT,       /* do read/write with iovec buf */
} qp_event_opt_t;


typedef union {
    qp_uchar_t*    block;
    struct iovec*  vector;
} qp_event_buf_t;


typedef struct qp_event_s*         qp_event_t;  


typedef struct {
    /** 
     * Read events process handler.
     * 
     * @param index Current event identification.
     * @param stat: Event stat. See qp_event_stat_t.
     * @param cache: Read cache address.
     * @param cache_size: Read cache content size.
     * @return Return QP_ERROR if the event should be shut down; return value > 0 
     *         means there are some data to be sent; otherwise return 0.
     */
    qp_int_t (*read)(qp_int_t index, qp_event_stat_t stat, \
        qp_uchar_t* cache, size_t cache_offset); 
    /**
     * Write events process handler.
     * 
     * @param index Current event identification.
     * @param stat: Event stat. See qp_event_stat_t.
     * @param cache: Write cache address.
     * @param write_bytes: Data size in cache that will be sent.(this arg should 
     *        always be set bigger than 0 if some data needs be sent)
     * @param cache_size: Max write cache size. The data to be sent should NOT bigger
     *         than it.
     * @return Return QP_ERROR will close current event immediately; return 0 will 
     *        close current event after data in cache is sent; otherwise will do 
     *        nothing except sending data.
     */
    qp_int_t (*write)(qp_int_t index, qp_event_stat_t stat, \
        qp_uchar_t* cache, size_t *write_bytes, size_t cache_size);
    
    void* (*idle)(void*); 
    void* idle_arg;
} qp_event_process_handle;


qp_event_t
qp_event_init(qp_event_t event, qp_int_t max_event_size, bool noblock, bool edge);


qp_int_t
qp_event_destroy(qp_event_t event);


qp_int_t
qp_event_regist_process_handler(qp_event_t event, qp_event_process_handle handle);


qp_int_t
qp_event_addevent(qp_event_t event, qp_int_t fd, qp_int_t timeout, bool listen, \
    bool auto_close);


qp_int_t
qp_event_dispatch(qp_event_t event, qp_int_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* QP_EVENT_H */
