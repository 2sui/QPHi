
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


#include "qp_o_typedef.h"
    
    
#ifdef  QP_OS_LINUX
    
#define  QP_EPOLL_ET           EPOLLET
#define  QP_EPOLL_ONESHOT      EPOLLONESHOT
#define  QP_EPOLL_IN           EPOLLIN               
#define  QP_EPOLL_OUT          EPOLLOUT
#define  QP_EPOLL_ERR          EPOLLERR              
#define  QP_EPOLL_HUP          EPOLLHUP             
#define  QP_EPOLL_RDHUP        EPOLLRDHUP 
#else

#define  QP_EPOLL_ET           1
#define  QP_EPOLL_ONESHOT      2
#define  QP_EPOLL_IN           3               
#define  QP_EPOLL_OUT          4
#define  QP_EPOLL_ERR          5              
#define  QP_EPOLL_HUP          6             
#define  QP_EPOLL_RDHUP        7 
#endif

#define  QP_EVENT_COMMONDATA_SIZE    256    // common buffer size
#define  QP_EVENT_TIMER_RESOLUTION   500    // ms
#define  QP_EVENT_TIMER_TIMEOUT      30000  // ms


/* buf type */
typedef enum  {
    QP_EVENT_BLOCK_OPT = 0,  /* do read/write with block buf */
    QP_EVENT_VECT_OPT,       /* do read/write with iovec buf */
} qp_event_opt_t;

/* event fd stat */
typedef enum {
    QP_EVENT_IDL = 0,
    QP_EVENT_NEW, /* event is new */
    QP_EVENT_PROCESS, /* event is running */
    QP_EVENT_CLOSE   /* event is closed */
} qp_event_stat_t;

typedef union {
    qp_uchar_t*    block;
    struct iovec*  vector;
} qp_event_buf_t;

typedef struct qp_event_fd_s*      qp_event_fd_t;
typedef  struct qp_event_data_s*   qp_event_data_t;
typedef struct qp_event_s*         qp_event_t;

/* init and destroy for qp_event_data_t */
typedef  void (*qp_event_opt_handler)(qp_event_data_t);
/* read/write handler */
typedef qp_int_t (*qp_read_handler)(qp_event_fd_t);
typedef qp_read_handler    qp_write_handler;
/* idle process handler */
typedef  void* (*qp_event_idle_handler)(void*); 
/* process handler */
typedef  qp_int_t (*qp_event_process_handler)(qp_event_data_t /* fd_data */, 
    qp_int_t /*fd*/, qp_event_stat_t /* stat */, bool /*read_finish*/, 
    size_t /* read_cnt */, bool /*write_finish*/, size_t /* write_cnt */);



inline bool
qp_event_is_alloced(qp_event_t evfd);

/****************************************************************************
 * It is recommended that one qp_event_t should be used by only one thread. *
 ****************************************************************************/

/**
 * Init a event module.
 * It need fd_size to tell this function the size of event bucket, and
 * if noblock is true the event loop will use noblock mode , and if edge is 
 * true the event loop will use ET mode.
 * init handler and destroy handler are handlers that how 
 * to init or destroy a qp_event_data_t for every event fd in event pool.
 * 
 * @param emodule: A new qp_event_t struct or NULL.
 * @param bucket_size: Event bucket size
 * @param resolution: Timer resolution.
 * @param init: Init handler for qp_event_data_t in event fd.
 * @param destroy: Destroy handler for qp_event_data_t in event fd.
 * @param noblock: Noblock mode.
 * @param edge: ET mode.
 * @param idle_cb: Handler runing during no event comming.
 * @param idle_arg: Argument for idle_cb.
 * @return If success return emodule pointer(if emodule is not NULL the return
 *     pointer is equal to it.), and return NULL if some error happen.
 */
qp_event_t
qp_event_init(qp_event_t emodule, qp_int_t bucket_size, qp_int_t resolution,
    qp_event_opt_handler init, qp_event_opt_handler destroy, 
    bool noblock, bool edge, void* (*idle_cb)(void *), void* idle_arg);

/**
 * Start event loop.You can controll when to quit the event loop by runstat.
 * 
 * @param emodule: Valid qp_event_t.
 * @param timeout: Event timeout.
 * @return If quit nomally it return QP_SUCCESS , otherwise return QP_ERROR.
 */
qp_int_t
qp_event_tiktok(qp_event_t emodule, qp_int_t timeout);

/**
 * Destory a mq_event handler.
 *
 * @param [emodule]  is the mq_event that will be destoryed (which has been inited).
 * @param [eventfd_destory_func]  Handler of how to destory mq_event_fd_t.
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
*/
qp_int_t
qp_event_destroy(qp_event_t emodule);

/**
 * Add events to this event module.
 */

/**
 * Add listen fd to event.
 * 
 * Note: You need add listen fd before calling qp_event_tiktok().
 * 
 * @param emodule: Valid qp_event_t.
 * @param fd: Socket or file description.
 * @param timeout:Event timeout.
 * @param listen: Listen mode.
 * @param auto_close: Auto close by module.
 * @return Return QP_SUCCESS if succes otherwise return QP_ERROR.
 */
qp_int_t
qp_event_addevent(qp_event_t emodule, qp_int_t fd, qp_int_t timeout,
    bool listen,bool auto_close);

//inline void
//qp_event_disable(qp_event_t emodule);

#ifdef __cplusplus
}
#endif

#endif 
