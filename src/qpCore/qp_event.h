
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
typedef  struct epoll_event    epoll_event_t;
#else
typedef  void    epoll_event_t;
#define          EPOLLIN        1
#define          EPOLLOUT       2
#define          EPOLLHUP       3
#endif

#define  QP_EVENT_SUCCESS            QP_SUCCESS
#define  QP_EVENT_ERROR              QP_ERROR
#define  QP_EVENT_COMMONDATA_SIZE    256


struct  qp_event_data_s {
    struct iovec*        readiovec;
    struct iovec*        writeiovec;
    qp_char_t*           readbuf;           /* user buffer */
    qp_char_t*           writebuf;
    qp_int_t             readiovec_size;
    qp_int_t             writeiovec_size;
    size_t               read_max;
    size_t               write_max;

    size_t               read_offset;       /* buf offset */
    size_t               write_offset;
    size_t               read_start;
    size_t               write_start;
    void*                data;              /* user data */
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
    qp_uint32_t            nativeclose:1;
    qp_uint32_t            peerclose:1;
    qp_uint32_t            write:1;       /* write event(set by do_mysel call) */
    qp_uint32_t            writehup:1;
    qp_uint32_t            read:1;        /* read event(set by EPOLLIN) */
    qp_uint32_t            readhup:1;
    qp_uint32_t            :22;
    qp_event_data_t        field;

    /**
     * When data recved or sent, do_myself will be called ,you decide how to do 
     * with current data. And do_myself also do the same thing.
     *
     * If the fd is closed by peer the [fd] will be passed with [-1].([readbuf 
     * and writebuf] is still the right buf linked to the fd, but the readoffset 
     * or writeoffset  will not change).
     *
     * [readstart],[writestart] and [writeoffset] can be changed by user.
     *
     * For read event:
     * [readbuf] is the read buffer which set by user, [readstart] can be 
     * changed by user for start index for next read event,  and [readoffset] 
     * is the offset index of last read event.[max_read_bucket] is the max 
     * buffer size, [readstart] should not bigger than that.
     *
     * For write event:
     * [writebuf] is the write buffer which set by user,[writestart]  and 
     * [writeoffset] can be changed by user to show where is the start index 
     * and where is the end index to be sent in [writebuf](start from index 
     * [writestart] and end before [writeoffset]),if [writebuf] is NULL , or 
     * [writestart] and [writeoffset] (which are passed in) are not equal,it 
     * means last write event has not finished.
     *
     * Note: If use readv or writev, you need to point the iovec of 
     * qp_event_data_t to your struct iovec[], then next read/write event 
     * will use readv or writev,otherwise you need to set iovec to NULL to tell
     * qp_event_tiktok to just use read or write function.
     *
     * For return:
     * Return QP_EVENT_SUCCESS or EPOLLIN or EPOLLOUT  or EPOLLHUP that means 
     * nothing to do(just wait for read event) or make up a write/read event 
     * or close the connection; return QP_EVENT_ERROR means some error happend 
     * and should close the connection.
     */
    qp_int_t
    (*do_myself)(qp_int_t  fd, qp_event_data_t*  data);

    qp_list_t                 ready_next;
};

typedef  struct qp_event_fd_s    qp_event_fd_t;


struct  qp_event_s {
    qp_fd_t                 evfd;                /* event module fd */
    
    qp_uint32_t             event_size;          /* event pool size */
    qp_uint32_t             event_listen_size;   /* listen event size */
    
    qp_uint32_t             available;           /* event number in pool */ 
    
    qp_pool_t               event_pool;      
    qp_list_t               ready;               /* event ready list */
    
    void*                   (*event_cb)(void*);  /* event call back when no event ready */
    void*                   event_cb_arg; 
    
    qp_int_t 
    (*event_fd_init_handler)(qp_event_data_t*, \
        qp_int_t (**)(qp_int_t, qp_event_data_t*), qp_int_t);
    
    qp_int_t 
    (*event_fd_destory_handler)(qp_event_data_t* edata);
    
    epoll_event_t           setter;   
    bool                    is_alloced; 
    qp_char_t               combuf[QP_EVENT_COMMONDATA_SIZE];
};

typedef  struct  qp_event_s    qp_event_t;


inline bool
qp_event_is_alloced(qp_event_t* evfd);


/**
 * Init qp_event module.If [fds] is NOT NULL which mean [fds] are all 
 * established fds and the [listenfds] event will be not available.
 * And if [listenfds] is set ,the fdsize must be bigger than 0 which means 
 * the pool size of  socket poll.
 *
 * @param [emodule] is qp_event_t handler that will be inited;
 * @param [flag] events flags that user needed.
 * @param [fds] is fd queue that have been inited;
 * @param [fdsize] is element number of fds;
 * @param [listenfds] is the element that used for socket accept event;
 * @param [listenfdsize] is the numer of listenfds elements;
 * @param [noblock]  the event is noblock;
 * @param [edge]   the event is oneshut;
 * @param [qp_event_fd_init_handler]  Function handler that use to init the 
 *            event fd elements in qp_event_t .Param [fd] is file descrpition 
 *            that need link to  and the [efd] is the mq_event_fd_t you need to 
 *            init. (Note: if fd in [fds] is unavailble this function will not 
 *            be called.)
 *
 * @param [wait_cb] is callback handler that will be called when waiting for 
 *            events(if NULL it will do nothing and block till events happend).
 *
 * @return If success return the count that inited elements, otherwise 
 *        return QP_EVENT_ERROR.
*/
qp_event_t*
qp_event_init(qp_event_t* emodule, 
    qp_int_t     flag,
    qp_fd_t*     listen_fds, 
    qp_uint32_t  listen_fd_size,
    qp_fd_t*     fds, 
    qp_uint32_t  fd_size,
    bool         noblock, 
    bool         edge,
    qp_int_t (*qp_event_fd_init_handler)(qp_event_data_t*,
        qp_int_t (**)(qp_int_t, qp_event_data_t*), qp_int_t),
    qp_int_t (*qp_event_fd_destroy_handler)(qp_event_data_t* edata),
    void*       (*wait_cb)(void *), 
    void*       wait_arg);


/**
 * Process event loop.
 *
 * @param [emodule] Inited qp_event_t struct.
 * @param [queue] Max event queue number.
 * @param [arg] Argurement that be passed into do_myself callback.
 * @param [wait_arg] Argument that be passed into waiting callback.
 * @param [runstat]   If bigger than 0 , run the event loop, otherwise quit.
 *
 * @return
 */
qp_int_t
qp_event_tiktok(qp_event_t* emodule, qp_int_t queue, qp_int_t *runstat);


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
