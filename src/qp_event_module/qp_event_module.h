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


#ifndef QP_EVENT_MODULE_H
#define QP_EVENT_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/qp_io_core.h"
#include "../core/qp_pool_core.h"
#include "../core/qp_debug.h"
#include "../qp_event.h"

#if !defined(QP_OS_LINUX)
typedef union epoll_data
{
  void         *ptr;
  qp_int_t     fd;
  qp_uint32_t  u32;
  qp_uint64_t  u64;
} epoll_data_t;

struct epoll_event
{
  qp_uint32_t   events;  /* Epoll events */
  epoll_data_t  data;    /* User data variable */
} __attribute__((__packed__));

enum EPOLL_CTL_OPTION {
    EPOLL_CTL_ADD,
    EPOLL_CTL_MOD,
    EPOLL_CTL_DEL
};

#endif

#if defined(QP_OS_LINUX)
typedef  struct epoll_event    qp_evpoll_event_s;
#else 
# if defined(QP_OS_BSD)
typedef  struct kevent         qp_evpoll_event_s;
# else
typedef  void                  qp_evpoll_event_s;
# endif
#endif

typedef  qp_evpoll_event_s*        qp_evpoll_event_t;
typedef struct qp_event_source_s*  qp_event_source_t;


typedef struct {
    qp_int_t (*qp_event_ev_create)(qp_event_t event, qp_int_t size);
    qp_int_t (*qp_event_ev_wait)(qp_event_t event, qp_int_t timeout);
    qp_int_t (*qp_event_ev_queue)();
    qp_int_t (*qp_event_ev_add)(qp_event_t event, qp_event_source_t source);
    qp_int_t (*qp_event_ev_reset)(qp_event_t event, qp_event_source_t source, \
        qp_uint32_t flag);
    qp_int_t (*qp_event_ev_del)(qp_event_t event, qp_event_source_t source);
} qp_event_ev_handle;


struct qp_event_source_s {
    struct qp_list_s           ready_next;          /* event source list */
    struct qp_rbtree_node_s    timer_node;    
    qp_uchar_t*                read_cache;          /* read/write cache */ 
    qp_uchar_t*                write_cache;
    size_t                     read_cache_size;
    size_t                     write_cache_size;
    size_t                     read_cache_offset;
    size_t                     write_cache_offset;
    size_t                     write_cache_cur_offset;
    qp_int_t                   source_fd;     
    qp_int_t                   index;
    qp_uint32_t                events;
    qp_uint32_t                revents;  
    qp_int_t                   timeout;
    
    qp_uint16_t                stat       :4;    
    qp_uint16_t                closed     :1;     /* need close */
    qp_uint16_t                listen     :1;     /* listen flag */
    qp_uint16_t                shutdown   :1;     /* source is closed */
    qp_uint16_t                urgen      :1;
    qp_uint16_t                write      :1;     /* write event */
    qp_uint16_t                read       :1;     /* read event */
    qp_uint16_t                write_close:1;
    qp_uint16_t                           :5;
    
    bool                       noblock;   /* need noblock */
    bool                       edge;      /* ET mod */
};


struct  qp_event_s {
    struct qp_pool_s         source_cache_pool; /* read/write cache pool */
    size_t                   source_cachepool_size; /* cache pool size */
    struct qp_pool_s         event_pool;        /* event mem pool */
    size_t                   eventpool_size;    /* event mem pool size */
    struct qp_rbtree_s       timer;
    struct qp_list_s         ready;             /* event ready list */
    struct qp_list_s         listen_ready;      /* event listen list */
    struct qp_fd_s           event_fd;     
    qp_event_ev_handle       event_handle;      /* event handler */
    qp_event_process_handle  process_handle;    /* process handler */
    qp_evpoll_event_t        bucket;            /* ready bucket */
    size_t                   bucket_size;       /* ready bucket size */
# if !defined(QP_OS_LINUX)
#  if defined(QP_OS_BSD)
    struct  kevent*          changelist;        /* change bucket */
    size_t                   changelist_size;
#  else 
    // other system
#  endif
# endif
    bool                     is_alloced; 
    bool                     is_run;
};

#ifdef __cplusplus
}
#endif

#endif /* QP_EVENT_MODULE_H */
