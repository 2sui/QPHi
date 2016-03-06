
/**
  * Copyright (C) 2sui.
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


/* buf type */
enum qp_event_opt_e {
    QP_EVENT_BLOCK_OPT = 0,  /* do read/write with block buf */
    QP_EVENT_VECT_OPT,       /* do read/write with iovec buf */
};

typedef enum qp_event_opt_e    qp_event_opt_t;


/* event fd stat */
enum qp_event_stat_e {
    QP_EVENT_IDL = 0,
    QP_EVENT_NEW, /* event is new */
    QP_EVENT_PROCESS, /* event is running */
    QP_EVENT_CLOSE   /* event is closed */
};

typedef  enum qp_event_stat_e   qp_event_stat_t;

typedef  qp_uint32_t            qp_event_done_t;
#define  QP_EVENT_NO_TIMEOUT    0
#define  QP_EVENT_READ_DONE     (1 < 0)
#define  QP_EVENT_WRITE_DONE    (1 < 1)

union qp_event_buf_s {
    qp_uchar_t*    block;
    struct iovec*  vector;
};

typedef union qp_event_buf_s    qp_event_buf_t;


typedef  struct qp_event_data_s    qp_event_data_t;
typedef  void (*qp_event_opt_handler)(qp_event_data_t*);
typedef  qp_int_t (*qp_event_process_handler)(qp_event_data_t* /* fd_data */, 
    qp_int_t /*fd*/, qp_event_stat_t /* stat */, bool /*read_finish*/, 
    size_t /* read_cnt */, bool /*write_finish*/, size_t /* write_cnt */);


struct  qp_event_data_s {
    /* read buf */
    qp_event_buf_t            readbuf; 
    /* write buf */
    qp_event_buf_t            writebuf;
    /* read buf max size/block */
    size_t                    readbuf_max; 
    /* read buf max size/block */
    size_t                    writebuf_max; 
    /* it will not call do_myself callback untill read_atleast bytes are read */
    size_t                    read_atleast; 
    /* it will not call do_myself callback untill write_atleast bytes are writen (it will not effect for now) */
    size_t                    write_atleast; 
    /* use block buf or iovec buf for next step */ 
    qp_event_opt_t            next_read_opt;    
    /* use block buf or iovec buf for next step */  
    qp_event_opt_t            next_write_opt; 
    /* event process handler for user */
    qp_event_process_handler  process_handler;
    /* user data */
    void*                     data;   /* user data */
};


struct qp_event_fd_s {
    qp_int_t               index;
    qp_int_t               efd;
    qp_int_t               eflag;
    qp_uint32_t            flag;          /* need close */
    qp_uint64_t            next_time;
    
    /* size that already read, it will be set when read done */  
    size_t                 read_done;     
    /* size that already writen, it will be set when write done */
    size_t                 write_done; 
    qp_list_t              ready_next;
    qp_event_data_t        field;
    
    /* event fd flags */
    qp_uint32_t            noblock:1;     /* need noblock */
    qp_uint32_t            edge:1;        /* ET mod */
    
    qp_uint32_t            listen:1;      /* is listen event */
    qp_uint32_t            closed:1;
    qp_uint32_t            stat:2;
    
    qp_uint32_t            nativeclose:1; /* native closed */
    qp_uint32_t            peerclose:1;   /* peer closed */
    qp_uint32_t            write:1;       
    qp_uint32_t            writehup:1;
    qp_uint32_t            write_finish:1;
    qp_uint32_t            read:1; 
    qp_uint32_t            readhup:1;
    qp_uint32_t            read_finish:1;
    
    qp_uint32_t            :20;
};

typedef  struct qp_event_fd_s    qp_event_fd_t;


struct  qp_event_s {
    qp_fd_t                 evfd;          /* event number in pool */ 
    qp_uint32_t             event_size;    /* event pool size */
    qp_pool_t               event_pool;    /* mem pool */       
    qp_list_t               ready;         /* event ready list */
    qp_list_t               listen_ready;
    qp_uint64_t             start_time;    /* timer begin */
    void*                   (*event_idle_cb)(void*);  /* idle event callback when no event ready */
    void*                   event_idle_cb_arg;   /* idle event callback arg */
    qp_event_opt_handler    init;
    qp_event_opt_handler    destroy;
    bool                    is_alloced; 
    bool                    is_run;
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
 * init handler and destroy handler are handlers that how 
 * to init or destroy a qp_event_data_t for every event fd in event pool.
 * 
 * If success return emodule pointer(if emodule is not NULL the return pointer is 
 * equal to it.), and return NULL if some error happen.
 */
qp_event_t*
qp_event_init(qp_event_t* emodule, qp_uint32_t fd_size, bool noblock, bool edge,
    qp_event_opt_handler init, qp_event_opt_handler destroy, 
    void* (*idle_cb)(void *), void* idle_arg);

/**
 * Start event loop.
 * You can controll when to quit the event loop by runstat.
 * 
 * If quit nomally it return QP_SUCCESS , otherwise return QP_ERROR.
 */
qp_int_t
qp_event_tiktok(qp_event_t* emodule, qp_int_t timeout);

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
qp_event_addevent(qp_event_t* emodule, qp_int_t fd, bool listen,bool auto_close);

inline void
qp_event_disable(qp_event_t* emodule);


#ifdef __cplusplus
}
#endif

#endif 
