
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
    
#define  QP_EPOLL_ET           EPOLLET
#define  QP_EPOLL_ONESHOT      EPOLLONESHOT
#define  QP_EPOLL_IN           EPOLLIN               
#define  QP_EPOLL_OUT          EPOLLOUT
#define  QP_EPOLL_ERR          EPOLLERR              
#define  QP_EPOLL_HUP          EPOLLHUP             
#define  QP_EPOLL_RDHUP        EPOLLRDHUP     
typedef  struct epoll_event    qp_epoll_event_t;

#else
typedef  void    qp_epoll_event_t;
#define          EPOLLIN        1
#define          EPOLLOUT       2
#define          EPOLLHUP       3
#endif

#define  QP_EVENT_COMMONDATA_SIZE    256


enum qp_event_opt_e {
    QP_EVENT_BLOCK_OPT = 0,  /* do read/write with block buf */
    QP_EVENT_VECT_OPT,       /* do read/write with iovec buf */
};

typedef enum qp_event_opt_e    qp_event_opt_t;


enum qp_event_stat_e {
    QP_EVENT_IDL = 0,
    QP_EVENT_NEW, /* event is new */
    QP_EVENT_PROCESS, /* event is running */
    QP_EVENT_CLOSE   /* event is closed */
};

typedef enum qp_event_stat_e    qp_event_stat_t;


struct qp_event_buf_s {
    qp_uchar_t*    block;
    struct iovec*  vector;
};

typedef struct qp_event_buf_s    qp_event_buf_t;


typedef  struct qp_event_data_s    qp_event_data_t;

struct  qp_event_data_s {
    /* read buf */
    qp_event_buf_t       readbuf; 
    /* write buf */
    qp_event_buf_t       writebuf;
    /* read buf max size/block */
    size_t               readbuf_max; 
    /* read buf max size/block */
    size_t               writebuf_max; 
    /* it will not call do_myself callback untill read_atleast bytes are read */
    size_t               read_atleast; 
    /* it will not call do_myself callback untill write_atleast bytes are writen (it will not effect for now) */
    size_t               write_atleast; 
    /* use block buf or iovec buf for next step */ 
    qp_event_opt_t       next_read_opt;    
    /* use block buf or iovec buf for next step */  
    qp_event_opt_t       next_write_opt;    
    qp_int_t
    (*process_handler)(qp_event_data_t*/* fd_data */, qp_event_stat_t/* stat */, 
        size_t /* read_cnt */, size_t /* write_cnt */);
    void*                data;   /* user data */
};


struct qp_event_fd_s {
    qp_int_t               index;
    qp_int_t               efd;
    qp_uint32_t            flag;      /* need close */
    qp_uint32_t            noblock:1;     /* need noblock */
    qp_uint32_t            edge:1;        /* ET mod */
    
    qp_uint32_t            listen:1;      /* is listen event */
    qp_uint32_t            closed:1;
           
    qp_uint32_t            process:1;
    qp_uint32_t            stat:2;
    
    qp_uint32_t            nativeclose:1; /* native closed */
    qp_uint32_t            peerclose:1;   /* peer closed */
    qp_uint32_t            write:1;       /* write event(set by do_mysel call) */
    qp_uint32_t            read:1; 
    
    qp_uint32_t            :20;
    /* size that already read, it will be set when read done */  
    size_t                 read_done;     
    /* size that already writen, it will be set when write done */
    size_t                 write_done; 
    qp_list_t              ready_next;
    qp_event_data_t        field;
};

typedef  struct qp_event_fd_s    qp_event_fd_t;


struct  qp_event_s {
    qp_fd_t                 evfd;         /* event module fd */
    qp_atom_t               available;    /* event number in pool */ 
    qp_uint32_t             event_size;   /* event pool size */
    qp_pool_t               event_pool;   /* mem pool */       
    qp_list_t               ready;    /* event ready list */
    qp_list_t               listen_ready;
    void*                   (*event_idle_cb)(void*);  /* idle event callback when no event ready */
    void*                   event_idle_cb_arg;   /* idle event callback arg */
    bool                    is_alloced; 
    /* read buf if user not assign */
    qp_uchar_t              combuf[QP_EVENT_COMMONDATA_SIZE];
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
qp_event_init(qp_event_t* emodule, qp_uint32_t fd_size, bool noblock, bool edge,
    void* (*idle_cb)(void *), void* idle_arg);

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

/**
 * Add listen fd to event.
 * 
 * Note: You need add listen fd before calling qp_event_tiktok().
 */
qp_int_t
qp_event_addevent(qp_event_t* evfd, qp_int_t listen);


#ifdef __cplusplus
}
#endif

#endif 
