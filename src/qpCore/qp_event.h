
/**
  * Copyright (C) sui
  * 
  * Event process module. 
  */


#ifndef QP_EVENT_H
#define QP_EVENT_H


#ifdef __cplusplus
extern "C" {
#endif


#include "qp_o_io.h"
#include "qp_pool.h"
    
    
#ifdef  QP_OS_LINUX
typedef  struct epoll_event    qp_epoll_event_t;
#else
typedef  void    qp_epoll_event_t;
#define          EPOLLIN        1
#define          EPOLLOUT       2
#define          EPOLLHUP       3
#endif

#define  QP_EVENT_COMMONDATA_SIZE    256

struct qp_event_buf_s {
    qp_uchar_t*    block;
    struct iovec*  vector;
};

typedef struct qp_event_buf_s    qp_event_buf_t;


enum qp_event_opt_e {
    QP_EVENT_BLOCK_OPT = 0,  /* do read/write with block buf */
    QP_EVENT_VECT_OPT,       /* do read/write with iovec buf */
};

typedef enum qp_event_opt_e    qp_event_opt_t;


struct  qp_event_data_s {
    qp_event_buf_t       readbuf;  /* read buf */
    qp_event_buf_t       writebuf; /* write buf */
    size_t               readbuf_max; /* read buf max size/block */
    size_t               writebuf_max; /* read buf max size/block */
    size_t               read_done; /* size that already read, it will be set when read done */      
    size_t               write_done; /* size that already writen, it will be set when write done */
    size_t               read_atleast; /* it will not call do_myself callback untill read_atleast bytes are read */
    size_t               write_atleast; /* it will not call do_myself callback untill write_atleast bytes are writen */
    qp_event_opt_t       next_read_opt;    /* use block buf or iovec buf for next step */ 
    qp_event_opt_t       next_write_opt;    /* use block buf or iovec buf for next step */  
    qp_int_t
    (*process_handler)(qp_event_data_t*, qp_int_t);
    void*                data;   /* user data */
};

typedef  struct qp_event_data_s    qp_event_data_t;


struct qp_event_fd_s {
    qp_int_t               index;
    qp_int_t               efd;
    qp_uint32_t            flag;
    qp_uint32_t            listen:1;      /* is listen event */
    qp_uint32_t            closed:1;      /* need close */
    qp_uint32_t            noblock:1;     /* need noblock */
    qp_uint32_t            edge:1;        /* ET mod */
    qp_uint32_t            nativeclose:1; /* native closed */
    qp_uint32_t            peerclose:1;   /* peer closed */
    qp_uint32_t            write:1;       /* write event(set by do_mysel call) */
    qp_uint32_t            writehup:1;    /* write done */
    qp_uint32_t            read:1;        /* read event(set by EPOLLIN) */
    qp_uint32_t            readhup:1;     /* read done */
    qp_uint32_t            :22;
    qp_list_t              ready_next;
    qp_event_data_t        field;
};

typedef  struct qp_event_fd_s    qp_event_fd_t;


struct  qp_event_s {
    qp_fd_t                 evfd;         /* event module fd */
    qp_uint32_t             event_size;   /* event pool size */
    qp_uint32_t             available;    /* event number in pool */ 
    qp_pool_t               event_pool;   /* mem pool */       
    qp_list_t               ready;    /* event ready list */
    void*                   (*event_idle_cb)(void*);  /* idle event callback when no event ready */
    void*                   event_idle_cb_arg;   /* idle event callback arg */
    
    /* qp_event_data_t init handler */
    qp_int_t 
    (*event_fd_init_handler)(qp_event_data_t*);
    
    /* qp_event_data_t destroy handler */
    qp_int_t 
    (*event_fd_destory_handler)(qp_event_data_t*); 
    
    bool                     is_alloced; 
    /* read buf if user not assign */
    qp_char_t                combuf[QP_EVENT_COMMONDATA_SIZE];
};

typedef  struct  qp_event_s    qp_event_t;


inline bool
qp_event_is_alloced(qp_event_t* evfd);

/**
 * It is recommended that one qp_event_t should be used by only one thread.
 */

/**
 * Init a event module.
 * It need fd_size to tell this function the size of event bucket, and
 * if noblock is true the event loop will use noblocking mode , and if edge is 
 * true the event loop will use ET mode.
 * qp_event_fd_init_handler and qp_event_fd_destroy_handler are handler that how 
 * to init or destroy a qp_event_data_t for every event fd in event pool.
 * 
 * If success return emodule pointer(if emodule is not NULL the return pointer is 
 * equal to it.), and return NULL if some error happen.
 */
qp_event_t*
qp_event_init(qp_event_t* emodule, qp_uint32_t fd_size, 
    qp_int_t (*qp_event_fd_init_handler)(qp_event_data_t*),
    qp_int_t (*qp_event_fd_destroy_handler)(qp_event_data_t*),
    bool noblock, bool edge, void* (*idle_cb)(void *), void* idle_arg);

/**
 * Start event loop.
 * You can controll when to quit the event loop by runstat.
 * 
 * If quit nomally it return QP_SUCCESS , otherwise return QP_ERROR.
 */
qp_int_t
qp_event_tiktok(qp_event_t* emodule, qp_int_t *runstat);

/**
 * Destory a mq_event handler.
 *
 * @param [emodule]  is the mq_event that will be destoryed (which has
 * been inited).
 * @param [eventfd_destory_func]  Handler of how to destory mq_event_fd_t.
*/
qp_int_t
qp_event_destroy(qp_event_t* emodule);

/**
 * Add events to this event module.
 */
qp_int_t
qp_event_add(qp_event_t* evfd, qp_event_fd_t* eventfd);

/**
 * Add listen fd to event.
 * 
 * Note: You need add listen fd before calling qp_event_tiktok().
 */
qp_int_t
qp_event_addon(qp_event_t* evfd, qp_int_t listen);

/**
 * Reset event.
 */
qp_int_t
qp_event_reset(qp_event_t* evfd, qp_event_fd_t* eventfd, qp_int_t flag);

/**
 * Delete event from event module.
 */
qp_int_t
qp_event_del(qp_event_t* evfd, qp_event_fd_t* eventfd);


#ifdef __cplusplus
}
#endif

#endif 
