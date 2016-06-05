
/**
  * Copyright (C) 2sui.
  */


#include "qp_event.h"
#include "qp_o_io.h"
#include "qp_o_pool.h"


#ifdef QP_OS_LINUX
typedef  struct epoll_event    qp_epoll_event_s;
#else 
typedef  void                  qp_epoll_event_s;
#endif

typedef  qp_epoll_event_s*       qp_epoll_event_t;

//struct  qp_event_data_s {
//    /* read buf */
//    qp_event_buf_t            readbuf; 
//    /* write buf */
//    qp_event_buf_t            writebuf;
//    /* read buf max size/block */
//    size_t                    readbuf_max; 
//    /* read buf max size/block */
//    size_t                    writebuf_max; 
//    /* it will not call do_myself callback untill read_atleast bytes are read */
//    size_t                    read_atleast; 
//    /* it will not call do_myself callback untill write_atleast bytes are writen (it will not effect for now) */
//    size_t                    write_atleast; 
//    /* use block buf or iovec buf for next step */ 
//    qp_event_opt_t            next_read_opt;    
//    /* use block buf or iovec buf for next step */  
//    qp_event_opt_t            next_write_opt; 
//    /* event process handler for user */ 
//    qp_event_process_handler  process_handler;
//    /* user data */
//    void*                     data;   /* user data */
//};

struct qp_event_fd_s {
    qp_int_t               index;
    qp_int_t               efd;
    qp_int_t               eflag;
    qp_uint32_t            flag;          /* need close */
    
    /* size that already read, it will be set when read done */  
    size_t                 read_done;     
    /* size that already writen, it will be set when write done */
    size_t                 write_done; 
    
    struct qp_list_s              ready_next;
    struct qp_event_data_s        field;
    struct qp_rbtree_node_s       timer_node;
    
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

struct  qp_event_s {
    struct qp_fd_s                 evfd;          /* event number in pool */
    qp_event_opt_handler    init;
    qp_event_opt_handler    destroy;
    qp_event_idle_handler   idle;
    void*                   idle_arg;   /* idle event callback arg */
    struct qp_pool_s               event_pool;    /* mem pool */ 
    struct qp_list_s               ready;         /* event ready list */
    struct qp_list_s               listen_ready;
    struct qp_rbtree_s             timer;
    qp_uint64_t             timer_begin;
    qp_int_t                timer_resolution;
    qp_int_t                timer_progress;
    qp_int_t                event_size;    /* event pool size */
    /* read buf if user not assign */
    qp_uchar_t              combuf[QP_EVENT_COMMONDATA_SIZE];
    bool                    timer_update;
    bool                    is_alloced; 
    bool                    is_run;
};

#define QP_EVENT_SET_ALLOCED(event)   (((qp_event_t)(event))->is_alloced=true)
#define QP_EVENT_UNSET_ALLOCED(event) (((qp_event_t)(event))->is_alloced=false)


//#define ENABLE_TIMER    


#ifndef  QP_OS_LINUX
qp_int_t
epoll_create(int __size)
{
    return -1;
}

qp_int_t
epoll_ctl(int __epfd, int __op, int __fd, epoll_event *__event)
{
    return -1;
}

qp_int_t
epoll_wait(int __epfd, epoll_event *__events, int __maxevents, int __timeout)
{
    return -1;
}

#endif


inline bool
qp_event_is_alloced(qp_event_t emodule) 
{ return emodule ? emodule->is_alloced : false; }

// MARK: private functions

/**
 * Update timer of event.
 */
qp_int_t
qp_event_timerevent(qp_event_t emodule, qp_event_fd_t eventfd, 
    qp_int_t timeout);

/**
 * Remove event from module.
 */
qp_int_t
qp_event_removeevent(qp_event_t emodule, qp_event_fd_t eventfd);

/**
 * Update current time.
 */
qp_int_t
qp_event_update_timer(qp_event_t emodule);

qp_int_t
qp_epoll_create(qp_event_t emodule, qp_int_t size);

qp_int_t
qp_epoll_wait(qp_event_t emodule, qp_epoll_event_t events, qp_int_t maxevents, 
    qp_int_t timeout);

/**
 * Add event to module.
 */
qp_int_t
qp_event_add(qp_event_t emodule, qp_event_fd_t eventfd);

/**
 * Reset event flag.
 */
qp_int_t
qp_event_reset(qp_event_t emodule, qp_event_fd_t eventfd, qp_int_t flag);

/**
 * Remove event from module.
 */
qp_int_t
qp_event_del(qp_event_t emodule, qp_event_fd_t eventfd);

/**
 * Accept socket event.
 */
qp_int_t
qp_event_accept(qp_event_fd_t eventfd);

/**
 * Close an event.
 */
qp_int_t
qp_event_close(qp_event_fd_t eventfd);

/**
 * Check if event should be closed.
 */
qp_int_t
qp_event_check_close(qp_event_fd_t eventfd);

/**
 * Write event.
 */
qp_int_t
qp_event_write(qp_event_fd_t eventfd);

qp_int_t
qp_event_writev(qp_event_fd_t eventfd);

/**
 * Read event.
 */
qp_int_t
qp_event_read(qp_event_fd_t eventfd);

qp_int_t
qp_event_readv(qp_event_fd_t eventfd);

/**
 * Reset event statu flag.
 */
void
qp_event_clear_flag(qp_event_fd_t eventfd);


qp_event_t
qp_event_create(qp_event_t emodule)
{
    if (NULL == emodule) {
        emodule = (qp_event_t)qp_alloc(sizeof(struct qp_event_s));
        
        if (NULL == emodule) {
            return NULL;
        }
        
        memset(emodule, 0, sizeof(struct qp_event_s));
        QP_EVENT_SET_ALLOCED(emodule);
        
    } else {
        memset(emodule, 0, sizeof(struct qp_event_s));
    }
    
    if (NULL == qp_fd_init(&emodule->evfd, QP_FD_TYPE_EVENT, false)) {
        qp_event_is_alloced(emodule) ? qp_free(emodule) : 1;
        return NULL;
    }
    
    qp_list_init(&emodule->ready);
    qp_list_init(&emodule->listen_ready);
    qp_rbtree_init(&emodule->timer);
    return emodule;
}

qp_event_t
qp_event_init(qp_event_t emodule, qp_int_t bucket_size, qp_int_t resolution,
    qp_event_opt_handler init, qp_event_opt_handler destroy, 
    bool noblock, bool edge, void* (*idle_cb)(void *), void* idle_arg)
{
    qp_int_t        mod = QP_EPOLL_IN /*| QP_EPOLL_OUT*/ | QP_EPOLL_RDHUP;
    qp_int_t        findex = 0;
    qp_event_fd_t   eventfd = NULL;
    
    if (1 > bucket_size) {
        return NULL;
    }
    
#ifndef QP_OS_LINUX
    return NULL;
#endif
    
    emodule = qp_event_create(emodule);
    
    if (NULL == emodule) {
        return NULL;
    }
    
    if (edge) {
        mod |= QP_EPOLL_ET /*| QP_EPOLL_ONESHOT */;
    }

    emodule->idle = idle_cb;
    emodule->idle_arg = idle_arg;
    emodule->init = init;
    emodule->destroy = destroy;
    emodule->event_size = bucket_size;
    emodule->timer_resolution = \
        (resolution > 0) ? resolution : QP_EVENT_TIMER_RESOLUTION;

    /* create epoll fd */
   if (QP_FD_INVALID == qp_epoll_create(emodule, 65535)) {
        qp_event_destroy(emodule);
        return NULL;
    }
    
    /* init event pool */
    if (NULL == qp_pool_init(&emodule->event_pool, sizeof(struct qp_event_fd_s), \
        emodule->event_size))
    {
        qp_event_destroy(emodule);
        return NULL;
    }
    
    /* init poollist */
    for (; findex < emodule->event_size; findex++) {   
        eventfd = (qp_event_fd_t)qp_pool_to_array(&emodule->event_pool, findex);
        eventfd->index = findex;
        eventfd->efd = QP_FD_INVALID;
        eventfd->eflag = 0;
        eventfd->flag = mod;
        eventfd->noblock = noblock;
        eventfd->edge = edge && QP_EPOLL_ET;
        qp_event_clear_flag(eventfd);
        qp_list_init(&eventfd->ready_next);
        memset(&eventfd->field, 0, sizeof(struct qp_event_data_s));
        memset(&eventfd->timer_node, 0, sizeof(struct qp_rbtree_node_s));
        
        if (emodule->init) {
            emodule->init(&eventfd->field);
        }
    }
    
    return emodule;
}

qp_int_t
qp_event_tiktok(qp_event_t emodule, qp_int_t timeout)
{
    qp_write_handler  write_handler;
    qp_read_handler   read_handler;
    qp_rbtree_node_t  tnode = NULL;
    qp_event_fd_t     eevent = NULL;
    qp_epoll_event_t  event_queue = NULL;
    ssize_t           ret = 0;
    qp_int_t          eevent_num = 0;
    qp_int_t          accept_fd = -1;
    qp_int_t          rflag = 0;
    qp_int_t          itr = 0;
    
#ifndef ENABLE_TIMER
    tnode = tnode;
#endif
    
    if (!emodule || !qp_fd_is_valid(&emodule->evfd)) {
        return QP_ERROR;
    }

    event_queue = qp_alloc(emodule->event_size * sizeof(qp_epoll_event_s));
    
    if (NULL == event_queue) {
        return QP_ERROR;
    }
    
    emodule->is_run = true;
    emodule->timer_update = true;
    
    /* event loop */
    while (emodule->is_run && qp_pool_used(&emodule->event_pool)) {
        
        /* clean timeout node */
#ifdef ENABLE_TIMER
        if (emodule->timer_update) {
            qp_event_update_timer(emodule);
            emodule->timer_update = false;
            emodule->timer_progress = 0;
            
            while (NULL != (tnode = qp_rbtree_min(&emodule->timer, NULL))) {
                
                if (tnode->key > emodule->timer_begin) {
                    break;
                }
                
                eevent = qp_rbtree_data(tnode, struct qp_event_fd_s, timer_node);
                qp_event_close(eevent);
                qp_event_removeevent(emodule, eevent);
            }
        }
#endif
         
        eevent_num = qp_epoll_wait(emodule, event_queue, emodule->event_size,\
            emodule->timer_resolution);

        /* do with error */
        if (1 > eevent_num) {

            if (-1 == eevent_num) {
                
                /* error quit */
                if (EINTR != errno) {
                    break;
                }
            }

            /* suspended by signal or no event happen */
            emodule->timer_update = true;
            
            /* no error beacuse no event happen */
            if (emodule->idle) {
                emodule->idle(emodule->idle_arg);
            }

            continue;
        }
        
        /* update timer if no timeout in epoll */
#ifdef ENABLE_TIMER
        if ((emodule->timer_resolution) <= emodule->timer_progress) {
            emodule->timer_update = true;
        }
#endif
         
        /* add event */
        for (itr = 0; itr < eevent_num; itr++) { 
            eevent = (qp_event_fd_t)(event_queue[itr].data.ptr);
            eevent->eflag = event_queue[itr].events;

            /* add read/write event to ready list */
            if (eevent->listen) {
                qp_list_push(&emodule->listen_ready, &eevent->ready_next);
                
            } else {
                qp_list_push(&emodule->ready, &eevent->ready_next);
            }
        }
        
        /* listen event */
        while (!qp_list_is_empty(&emodule->listen_ready)) {
            eevent = qp_list_data(qp_list_first(&emodule->listen_ready), \
                struct qp_event_fd_s, ready_next);
            qp_list_pop(&emodule->listen_ready);
            
            do {
                accept_fd = qp_event_accept(eevent);
                
                /* accept error */
                if (QP_FD_INVALID == accept_fd) {
                    
                    if (!(EAGAIN == errno || EWOULDBLOCK == errno || 
                        EINTR == errno))
                    {
                        qp_event_close(eevent);
                        qp_event_removeevent(emodule, eevent);
                    }
                    
                    break;
                } 
                
                if (QP_ERROR == \
                    qp_event_addevent(emodule, accept_fd, timeout, false, true))
                {
                    close(accept_fd);
                }

            } while (eevent->edge);
 
        }
            
        /* do read/write event */
        while (!qp_list_is_empty(&emodule->ready)) {
            eevent = qp_list_data(qp_list_first(&emodule->ready), struct qp_event_fd_s,\
                ready_next);
            qp_list_pop(&emodule->ready);
            
            eevent->nativeclose |= (QP_EPOLL_HUP | QP_EPOLL_ERR) & eevent->eflag;
            eevent->peerclose |= QP_EPOLL_RDHUP & eevent->eflag;
            eevent->write &= !eevent->peerclose;
            eevent->read = QP_EPOLL_IN & eevent->eflag;

            /* if no event */
            if (!(eevent->read | eevent->write | eevent->nativeclose)) {
                continue;
            }
            
            qp_event_timerevent(emodule, eevent, timeout);
            
            write_handler = (QP_EVENT_BLOCK_OPT == eevent->field.next_write_opt)?\
                qp_event_write : qp_event_writev;
            read_handler = (QP_EVENT_BLOCK_OPT == eevent->field.next_read_opt) ? \
                qp_event_read : qp_event_readv;

            /* do with write/read events */
            do {
                /* connection closed */
                if (QP_ERROR == qp_event_check_close(eevent)) {
                    break;
                }
                
                /* if all events have done */
                if (!(eevent->read | eevent->write)) {
                    break; 
                }
                
                /* write event */
                if (eevent->write) {
                    
                    if (eevent->field.writebuf.block 
                        && (QP_ERROR == write_handler(eevent))) 
                    {
                        break;
                    }
                }
                
                /* read event */
                if (eevent->read) {
                    
                    if (!eevent->field.readbuf.block) {
                        eevent->field.readbuf.block = emodule->combuf;
                        eevent->field.readbuf_max = QP_EVENT_COMMONDATA_SIZE;
                    }
                    
                    ret = read_handler(eevent);
                    
                    if (eevent->field.readbuf.block == emodule->combuf) {
                        eevent->field.readbuf.block = NULL;
                        eevent->field.readbuf_max = 0;
                    }
                    
                    if (QP_ERROR == ret) {
                        break;
                    }
                }
                
            } while (eevent->edge);
            
            /* if read/write event was hupped */
            if ((QP_FD_INVALID != eevent->efd) 
                && (eevent->writehup || eevent->readhup)) 
            {   
                rflag = eevent->writehup ? QP_EPOLL_OUT : 0;
                qp_event_reset(emodule, eevent, rflag);
                eevent->writehup = 0;
                eevent->write = 1;
            }

            /* stat change */
            if (QP_FD_INVALID == eevent->efd) {
                eevent->stat = QP_EVENT_CLOSE;    
            }
            
            /* process the events */
            if (eevent->write_finish || eevent->read_finish) {
                
                if (eevent->field.process_handler) {
                    ret = eevent->field.process_handler(&eevent->field,
                    eevent->efd, eevent->stat, eevent->read_finish, 
                    eevent->read_done,eevent->write_finish, eevent->write_done);
                
                    if (QP_FD_INVALID != eevent->efd) {
                    
                        if (QP_SUCCESS > ret) {
                            qp_event_close(eevent);
                        
                        } else {
                        
                            if (ret & QP_EPOLL_OUT) {
                                qp_event_reset(emodule, eevent, QP_EPOLL_OUT);
                                eevent->write = 1;
                                eevent->write_done = 0;
                            }
                        }
                    }
                }
                
                if (eevent->read_finish) {
                    eevent->read_done = 0;
                    eevent->read_finish = 0;
                }
                
                if (eevent->write_finish) {
                    eevent->write_finish = 0;   
                }
            }
            
            /* stat change */
            if (QP_EVENT_NEW == eevent->stat) {
                eevent->stat = QP_EVENT_PROCESS;
            }
            
            if (QP_FD_INVALID == eevent->efd) {
                qp_event_removeevent(emodule, eevent);
            }
            
        }  // while
        
    }  // while

    emodule->is_run = false;
    qp_free(event_queue);
    return QP_SUCCESS;
}


qp_int_t
qp_event_destroy(qp_event_t emodule)
{
    if (emodule && qp_fd_is_inited(&emodule->evfd) && !emodule->is_run) { 
        qp_int_t i = 0;
        qp_event_fd_t eventfd = NULL;
        
        qp_fd_destroy(&emodule->evfd);
        
        for (; i < emodule->event_size; i++) {
            eventfd = (qp_event_fd_t)qp_pool_to_array(&emodule->event_pool, i);
            
            if (QP_FD_INVALID != eventfd->efd) {
                qp_event_close(eventfd);
            }
            
            if (emodule->destroy) {
                emodule->destroy(&eventfd->field);
            }
        }

        qp_pool_destroy(&emodule->event_pool, true);
        
        if (qp_event_is_alloced(emodule)) {
            qp_free(emodule);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_event_addevent(qp_event_t emodule, qp_int_t fd, qp_int_t timeout,
    bool listen, bool auto_close)
{ 
    qp_event_fd_t revent = NULL;
    
    if (!emodule) {
        return QP_ERROR;
    }
    
    if (qp_pool_available(&emodule->event_pool) && (fd > QP_FD_INVALID)) {
        /* get the first idle element */
        revent = (qp_event_fd_t)qp_pool_alloc(&emodule->event_pool, \
            sizeof(struct qp_event_fd_s));
        revent->efd = fd;

        /* add event to pool */
        if (QP_ERROR == qp_event_add(emodule, revent)) {
            revent->efd = QP_FD_INVALID;
//            qp_event_clear_flag(revent);
            qp_pool_free(&emodule->event_pool, revent);
            return QP_ERROR;
        }
                        
        revent->closed = auto_close;
        revent->listen = listen;
        
        if (revent->listen) {
            revent->stat = QP_EVENT_PROCESS;
            revent->timer_node.key = 0xffffffffffffffff;
            
        } else {
            revent->stat = QP_EVENT_NEW;
#ifdef ENABLE_TIMER
            revent->timer_node.key = emodule->timer_begin + \
                ((timeout > 0) ? timeout : QP_EVENT_TIMER_TIMEOUT) * 1000 + \
                (++emodule->timer_progress);
            
            
            if (!qp_rbtree_insert(&emodule->timer, &revent->timer_node)) {
                qp_event_del(emodule, revent);
                qp_event_clear_flag(revent);
                qp_pool_free(&emodule->event_pool, revent);
                return QP_ERROR;
            }
#else
            timeout = timeout;
#endif
        }
        
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}

qp_int_t
qp_event_timerevent(qp_event_t emodule, qp_event_fd_t eventfd, 
    qp_int_t timeout)
{
#ifdef ENABLE_TIMER
    return QP_SUCCESS;
#endif
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_rbtree_delete(&emodule->timer, &eventfd->timer_node);
    eventfd->timer_node.key = emodule->timer_begin + \
        ((timeout > 0) ? timeout : QP_EVENT_TIMER_TIMEOUT) * 1000 + \
        (++emodule->timer_progress);
    qp_rbtree_insert(&emodule->timer, &eventfd->timer_node);
     
    return QP_SUCCESS;
}

qp_int_t
qp_event_removeevent(qp_event_t emodule, qp_event_fd_t eventfd)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
#ifdef ENABLE_TIMER
    if (!eventfd->listen) {
        qp_rbtree_delete(&emodule->timer, &eventfd->timer_node);
    }
#endif
     
    qp_event_del(emodule, eventfd);
    qp_event_clear_flag(eventfd);
    qp_pool_free(&emodule->event_pool, eventfd);
    return QP_SUCCESS;
}

inline void
qp_event_disable(qp_event_t emodule)
{ emodule ? emodule->is_run = false : 1; }


qp_int_t
qp_event_update_timer(qp_event_t emodule)
{
#ifdef ENABLE_TIMER 
    return QP_SUCCESS;
#endif
    if (!emodule) {
        return QP_ERROR;
    }
    
    struct timeval etime;
    gettimeofday(&etime, NULL);
    emodule->timer_begin = etime.tv_sec * 1000000 + etime.tv_usec;
    return QP_SUCCESS;
}

qp_int_t
qp_epoll_create(qp_event_t emodule, qp_int_t size)
{
    if (!emodule) {
        return QP_ERROR;
    }
    
#ifdef QP_OS_LINUX
    emodule->evfd.fd = epoll_create(size);
#else 
    emodule->evfd.fd = QP_ERROR;
#endif
    return emodule->evfd.fd;
}

qp_int_t
qp_epoll_wait(qp_event_t emodule, qp_epoll_event_t events, qp_int_t maxevents, 
    qp_int_t timeout)
{
    if (!emodule || !events) {
        return QP_ERROR;
    }
    
#ifdef QP_OS_LINUX
    return epoll_wait(emodule->evfd.fd, events, maxevents, 
        timeout);
#else 
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_add(qp_event_t emodule, qp_event_fd_t eventfd)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
    
    if (eventfd->noblock) {
        fcntl(eventfd->efd, F_SETFL, fcntl(eventfd->efd, F_GETFL) | O_NONBLOCK);
    }
    
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = eventfd->flag;
    return epoll_ctl(emodule->evfd.fd, EPOLL_CTL_ADD, eventfd->efd, &setter);
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_reset(qp_event_t emodule, qp_event_fd_t eventfd, qp_int_t flag)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = eventfd->flag | flag;
    return epoll_ctl(emodule->evfd.fd, EPOLL_CTL_MOD, 
        eventfd->efd, &setter);
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_del(qp_event_t emodule, qp_event_fd_t eventfd)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = 0;
    return epoll_ctl(emodule->evfd.fd, EPOLL_CTL_DEL, eventfd->efd, &setter);
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_accept(qp_event_fd_t eventfd)
{
    return eventfd ? accept(eventfd->efd, NULL, NULL) : QP_ERROR;
}

qp_int_t
qp_event_close(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return QP_ERROR;
    }
    
    if (eventfd->closed && (QP_FD_INVALID != eventfd->efd)) {
        close(eventfd->efd);
    }

    eventfd->efd = QP_FD_INVALID;
    return QP_SUCCESS;
}


qp_int_t
qp_event_check_close(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return QP_ERROR;
    }
    
    if (eventfd->nativeclose) {
        qp_event_close(eventfd);
        return QP_ERROR;
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_write(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return QP_ERROR;
    }
    
    size_t rest;
    ssize_t ret;
    
    if (eventfd->write && (QP_FD_INVALID != eventfd->efd)) {

        if (eventfd->write_done < eventfd->field.writebuf_max) {
            rest = eventfd->field.writebuf_max - eventfd->write_done;
            ret = write(eventfd->efd, eventfd->field.writebuf.block + \
                eventfd->write_done, rest);

            if (1 > ret) {
                eventfd->write = 0;
                
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    /* need close */
                    qp_event_close(eventfd);
                    eventfd->write_finish = 1;
                    return QP_ERROR;
                }

                /* write buf full and write event hup */
                eventfd->writehup = 1;
                
            } else {
                eventfd->write_done = eventfd->write_done + ret;
            }

        } else {
            /* write finish */
            eventfd->write = 0;
            eventfd->write_finish = 1;
        }
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_writev(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return QP_ERROR;
    }
    
    ssize_t ret;

    if (eventfd->write && eventfd->field.writebuf_max 
        && (QP_FD_INVALID != eventfd->efd)) 
    {
        ret = writev(eventfd->efd, eventfd->field.writebuf.vector, \
            eventfd->field.writebuf_max);
        eventfd->write = 0;
        eventfd->write_finish = 1;

        if (1 > ret) {

            if ((0 == ret) 
                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
            {
                qp_event_close(eventfd);
                return QP_ERROR;
            }
        }
    }
    
    return QP_SUCCESS;
}

qp_int_t
qp_event_read(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return QP_ERROR;
    }
    
    size_t rest;
    ssize_t ret;
    
    if (eventfd->field.read_atleast > eventfd->field.readbuf_max) {
        eventfd->field.read_atleast = eventfd->field.readbuf_max;
    }

    if (eventfd->read && (QP_FD_INVALID != eventfd->efd)) {

        if (eventfd->read_done < eventfd->field.readbuf_max) {
            rest = eventfd->field.readbuf_max - eventfd->read_done;
            ret = read(eventfd->efd, eventfd->field.readbuf.block + \
                eventfd->read_done, rest);

            if (1 > ret) {
                eventfd->read = 0;

                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    qp_event_close(eventfd);
                    eventfd->read_finish = 1;
                    return QP_ERROR;
                }
                
                if (eventfd->field.read_atleast) {
                    eventfd->read_finish = \
                        eventfd->field.read_atleast > eventfd->read_done ? 0 : 1;
                    
                } else {
                    eventfd->read_finish = 1;
                }

            } else {
                eventfd->read_done = eventfd->read_done + ret;
            }

        } else {
            eventfd->read = 0;
            eventfd->read_finish = 1;

            if (eventfd->edge) {
                /* we will rest beacues kernel buf still have data */
                eventfd->readhup = 1;
            }
        }
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_readv(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return QP_ERROR;
    }
    
    ssize_t ret;

    if (eventfd->read && eventfd->field.readbuf_max 
        && (QP_FD_INVALID != eventfd->efd)) 
    {
        ret = readv(eventfd->efd, eventfd->field.readbuf.vector, 
            eventfd->field.readbuf_max);
        eventfd->read = 0;
        eventfd->read_finish = 1;

        if (1 > ret) {

            if ((0 == ret) 
                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
            {
                qp_event_close(eventfd);
                return QP_ERROR;
            }
        }
    }
    

    return QP_SUCCESS;
}

void
qp_event_clear_flag(qp_event_fd_t eventfd)
{
    if (!eventfd) {
        return;
    }
    
    eventfd->listen = 0;
    eventfd->closed = 0;
    eventfd->stat = QP_EVENT_IDL;
    eventfd->nativeclose = 0;
    eventfd->peerclose = 0;
    eventfd->write = 0;
    eventfd->writehup = 0;
    eventfd->write_finish = 0;
    eventfd->read = 0;
    eventfd->readhup = 0;
    eventfd->read_finish = 0;
    eventfd->read_done = 0;
    eventfd->write_done = 0;
}
