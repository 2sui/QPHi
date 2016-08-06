
/**
  * Copyright (C) 2sui.
  * 
  * Event process module. 
  */


#ifndef QP_EVENT_H
#define QP_EVENT_H


#include "qp_o_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
    
# ifdef  QP_OS_LINUX
#  define  QP_EPOLL_ET           EPOLLET
#  define  QP_EPOLL_ONESHOT      EPOLLONESHOT
#  define  QP_EPOLL_IN           EPOLLIN               
#  define  QP_EPOLL_OUT          EPOLLOUT
#  define  QP_EPOLL_ERR          EPOLLERR              
#  define  QP_EPOLL_HUP          EPOLLHUP             
#  define  QP_EPOLL_RDHUP        EPOLLRDHUP 
# else
#  define  QP_EPOLL_ET           1
#  define  QP_EPOLL_ONESHOT      2
#  define  QP_EPOLL_IN           3               
#  define  QP_EPOLL_OUT          4
#  define  QP_EPOLL_ERR          5              
#  define  QP_EPOLL_HUP          6             
#  define  QP_EPOLL_RDHUP        7 
# endif

#define  QP_EVENT_READCACHE_SIZE    4096    // common buffer size
    
/* buf type */
typedef enum  {
    QP_EVENT_BLOCK_OPT = 0,  /* do read/write with block buf */
    QP_EVENT_VECT_OPT,       /* do read/write with iovec buf */
} qp_event_opt_t;

/* event fd stat */
typedef enum {
    QP_EVENT_IDL = 0,
    QP_EVENT_NEW = 1<<0, /* event is new */
    QP_EVENT_PROCESS = 1<<1, /* event is running */
    QP_EVENT_CLOSE = 1<<2   /* event is closed */
} qp_event_stat_t;

typedef union {
    qp_uchar_t*    block;
    struct iovec*  vector;
} qp_event_buf_t;

typedef struct qp_event_s*         qp_event_t;

/* idle process handler */
typedef  void* (*qp_event_idle_handler)(void*); 
/* process handler */
typedef  qp_int_t (*qp_event_process_handler)(qp_event_data_t /* fd_data */, 
    qp_int_t /*fd*/, qp_event_stat_t /* stat */, bool /*read_finish*/, 
    size_t /* read_cnt */, bool /*write_finish*/, size_t /* write_cnt */);


inline bool
qp_event_is_alloced(qp_event_t evfd);

qp_event_t
qp_event_init(qp_event_t event, qp_int_t max_event_size, bool noblock, bool edge);

qp_int_t
qp_event_tiktok(qp_event_t emodule, qp_int_t timeout);

qp_int_t
qp_event_destroy(qp_event_t event);

qp_int_t
qp_event_regist_idle_handler(qp_event_t event, qp_event_idle_handler idle_cb, \
    void* idle_arg);

qp_int_t
qp_event_addevent(qp_event_t event, qp_int_t fd, qp_int_t timeout, bool listen, \
    bool auto_close);

qp_int_t
qp_event_removeevent(qp_event_t event, qp_event_source_t source);

qp_int_t
qp_event_dispatch(qp_event_t event, qp_int_t timeout);
        
#ifdef __cplusplus
}
#endif

#endif 
