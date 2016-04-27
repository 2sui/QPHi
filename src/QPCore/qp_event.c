
/**
  * Copyright (C) 2sui.
  */


#include "qp_event.h"
#include "qp_pool.h"


#define QP_EVENT_SET_ALLOCED(event)   (((qp_event_t*)(event))->is_alloced=true)
#define QP_EVENT_UNSET_ALLOCED(event) (((qp_event_t*)(event))->is_alloced=false)


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
qp_event_is_alloced(qp_event_t* emodule) 
{ return emodule ? emodule->is_alloced : false; }

// MARK: private functions

/**
 * Update timer of event.
 */
qp_int_t
qp_event_timerevent(qp_event_t* emodule, qp_event_fd_t* eventfd, 
    qp_int_t timeout);

/**
 * Remove event from module.
 */
qp_int_t
qp_event_removeevent(qp_event_t* emodule, qp_event_fd_t* eventfd);

/**
 * Update current time.
 */
qp_int_t
qp_event_update_timer(qp_event_t* emodule);

qp_int_t
qp_epoll_create(qp_event_t* emodule, qp_int_t size);

qp_int_t
qp_epoll_wait(qp_event_t* emodule, qp_epoll_event_t *events, qp_int_t maxevents, 
    qp_int_t timeout);

/**
 * Add event to module.
 */
qp_int_t
qp_event_add(qp_event_t* emodule, qp_event_fd_t* eventfd);

/**
 * Reset event flag.
 */
qp_int_t
qp_event_reset(qp_event_t* emodule, qp_event_fd_t* eventfd, qp_int_t flag);

/**
 * Remove event from module.
 */
qp_int_t
qp_event_del(qp_event_t* emodule, qp_event_fd_t* eventfd);

/**
 * Accept socket event.
 */
qp_int_t
qp_event_accept(qp_event_fd_t* eventfd);

/**
 * Close an event.
 */
qp_int_t
qp_event_close(qp_event_fd_t* eventfd);

/**
 * Check if event should be closed.
 */
qp_int_t
qp_event_check_close(qp_event_fd_t* eventfd);

/**
 * Write event.
 */
qp_int_t
qp_event_write(qp_event_fd_t* eventfd);

qp_int_t
qp_event_writev(qp_event_fd_t* eventfd);

/**
 * Read event.
 */
qp_int_t
qp_event_read(qp_event_fd_t* eventfd);

qp_int_t
qp_event_readv(qp_event_fd_t* eventfd);

/**
 * Reset event statu flag.
 */
void
qp_event_clear_flag(qp_event_fd_t* eventfd);


qp_event_t*
qp_event_create(qp_event_t* emodule)
{
    if (NULL == emodule) {
        emodule = (qp_event_t*)qp_alloc(sizeof(qp_event_t));
        
        if (NULL == emodule) {
            return NULL;
        }
        
        memset(emodule, 0, sizeof(qp_event_t));
        QP_EVENT_SET_ALLOCED(emodule);
        
    } else {
        memset(emodule, 0, sizeof(qp_event_t));
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

qp_event_t*
qp_event_init(qp_event_t* emodule, qp_int_t bucket_size, qp_int_t resolution,
    qp_event_opt_handler init, qp_event_opt_handler destroy, 
    bool noblock, bool edge, void* (*idle_cb)(void *), void* idle_arg)
{
    qp_int_t        mod = QP_EPOLL_IN /*| QP_EPOLL_OUT*/ | QP_EPOLL_RDHUP;
    qp_int_t        findex = 0;
    qp_event_fd_t*  eventfd = NULL;
    
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
    if (NULL == qp_pool_init(&emodule->event_pool, sizeof(qp_event_fd_t), \
        emodule->event_size))
    {
        qp_event_destroy(emodule);
        return NULL;
    }
    
    /* init poollist */
    for (; findex < emodule->event_size; findex++) {   
        eventfd = (qp_event_fd_t*)qp_pool_to_array(&emodule->event_pool, findex);
        eventfd->index = findex;
        eventfd->efd = QP_FD_INVALID;
        eventfd->eflag = 0;
        eventfd->flag = mod;
        eventfd->noblock = noblock;
        eventfd->edge = edge && QP_EPOLL_ET;
        qp_event_clear_flag(eventfd);
        qp_list_init(&eventfd->ready_next);
        memset(&eventfd->field, 0, sizeof(qp_event_data_t));
        memset(&eventfd->timer_node, 0, sizeof(qp_rbtree_node_t));
        
        if (emodule->init) {
            emodule->init(&eventfd->field);
        }
    }
    
    return emodule;
}

qp_int_t
qp_event_tiktok(qp_event_t *emodule, qp_int_t timeout)
{
    qp_write_handler  write_handler;
    qp_read_handler   read_handler;
    qp_rbtree_node_t* tnode = NULL;
    qp_event_fd_t*    eevent = NULL;
    qp_epoll_event_t* event_queue = NULL;
    ssize_t           ret = 0;
    qp_int_t          eevent_num = 0;
    qp_int_t          accept_fd = -1;
    qp_int_t          rflag = 0;
    qp_int_t          itr = 0;
    
    if (!emodule || !qp_fd_is_valid(&emodule->evfd)) {
        return QP_ERROR;
    }

    event_queue = qp_alloc(emodule->event_size * sizeof(qp_epoll_event_t));
    
    if (NULL == event_queue) {
        return QP_ERROR;
    }
    
    emodule->is_run = true;
    emodule->timer_update = true;
    
    /* event loop */
    while (emodule->is_run && qp_pool_used(&emodule->event_pool)) {
        
        /* clean timeout node */
        
        if (emodule->timer_update) {
            qp_event_update_timer(emodule);
            emodule->timer_update = false;
            emodule->timer_progress = 0;
            
            while (NULL != (tnode = qp_rbtree_min(&emodule->timer, NULL))) {
                
                if (tnode->key > emodule->timer_begin) {
                    break;
                }
                
                eevent = qp_rbtree_data(tnode, qp_event_fd_t, timer_node);
                qp_event_close(eevent);
                qp_event_removeevent(emodule, eevent);
            }
        }
         
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
        if ((emodule->timer_resolution * 8) <= emodule->timer_progress) {
            emodule->timer_update = true;
        }
         
        /* add event */
        for (itr = 0; itr < eevent_num; itr++) { 
            eevent = (qp_event_fd_t*)(event_queue[itr].data.ptr);
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
                qp_event_fd_t, ready_next);
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
            eevent = qp_list_data(qp_list_first(&emodule->ready), qp_event_fd_t,\
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
qp_event_destroy(qp_event_t *emodule)
{
    if (emodule && qp_fd_is_inited(&emodule->evfd) && !emodule->is_run) { 
        qp_int_t i = 0;
        qp_event_fd_t* eventfd = NULL;
        
        qp_fd_destroy(&emodule->evfd);
        
        for (; i < emodule->event_size; i++) {
            eventfd = (qp_event_fd_t*)qp_pool_to_array(&emodule->event_pool, i);
            
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
qp_event_addevent(qp_event_t* emodule, qp_int_t fd, qp_int_t timeout,
    bool listen, bool auto_close)
{ 
    qp_event_fd_t* revent = NULL;
    
    if (!emodule) {
        return QP_ERROR;
    }
    
    if (qp_pool_available(&emodule->event_pool) && (fd > QP_FD_INVALID)) {
        /* get the first idle element */
        revent = (qp_event_fd_t*)qp_pool_alloc(&emodule->event_pool, \
            sizeof(qp_event_fd_t));
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
            revent->timer_node.key = emodule->timer_begin + \
                ((timeout > 0) ? timeout : QP_EVENT_TIMER_TIMEOUT) + \
                (++emodule->timer_progress) / 8;
            
            
            if (!qp_rbtree_insert(&emodule->timer, &revent->timer_node)) {
                qp_event_del(emodule, revent);
                qp_event_clear_flag(revent);
                qp_pool_free(&emodule->event_pool, revent);
                return QP_ERROR;
            }
             
        }
        
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}

qp_int_t
qp_event_timerevent(qp_event_t* emodule, qp_event_fd_t* eventfd, 
    qp_int_t timeout)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_rbtree_delete(&emodule->timer, &eventfd->timer_node);
    eventfd->timer_node.key = emodule->timer_begin + \
        ((timeout > 0) ? timeout : QP_EVENT_TIMER_TIMEOUT) + \
        (++emodule->timer_progress) / 8;
    qp_rbtree_insert(&emodule->timer, &eventfd->timer_node);
     
    return QP_SUCCESS;
}

qp_int_t
qp_event_removeevent(qp_event_t* emodule, qp_event_fd_t* eventfd)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    if (!eventfd->listen) {
        qp_rbtree_delete(&emodule->timer, &eventfd->timer_node);
    }
     
    qp_event_del(emodule, eventfd);
    qp_event_clear_flag(eventfd);
    qp_pool_free(&emodule->event_pool, eventfd);
    return QP_SUCCESS;
}

inline void
qp_event_disable(qp_event_t* emodule)
{ emodule ? emodule->is_run = false : 1; }


qp_int_t
qp_event_update_timer(qp_event_t* emodule)
{
    if (!emodule) {
        return QP_ERROR;
    }
    
    struct timeval etime;
    gettimeofday(&etime, NULL);
    emodule->timer_begin = etime.tv_sec * 1000 + etime.tv_usec / 1000;
    return QP_SUCCESS;
}

qp_int_t
qp_epoll_create(qp_event_t* emodule, qp_int_t size)
{
    if (!emodule) {
        return QP_ERROR;
    }
    
#ifdef QP_OS_LINUX
    emodule->evfd.retsno = epoll_create(size);
#else 
    emodule->evfd.retsno = QP_ERROR;
#endif
    emodule->evfd.errono = errno;
    emodule->evfd.fd = emodule->evfd.retsno;
    return emodule->evfd.retsno;
}

qp_int_t
qp_epoll_wait(qp_event_t* emodule, qp_epoll_event_t *events, qp_int_t maxevents, 
    qp_int_t timeout)
{
    if (!emodule || !events) {
        return QP_ERROR;
    }
    
#ifdef QP_OS_LINUX
    emodule->evfd.retsno = epoll_wait(emodule->evfd.fd, events, maxevents, 
        timeout);
#else 
    emodule->evfd.retsno = QP_ERROR;
#endif
    emodule->evfd.errono = errno;
    return emodule->evfd.retsno;
}

qp_int_t
qp_event_add(qp_event_t *emodule, qp_event_fd_t *eventfd)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_epoll_event_t setter;
    
    if (eventfd->noblock) {
        fcntl(eventfd->efd, F_SETFL, fcntl(eventfd->efd, F_GETFL) | O_NONBLOCK);
    }
    
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = eventfd->flag;
    emodule->evfd.retsno = epoll_ctl(emodule->evfd.fd, EPOLL_CTL_ADD, 
        eventfd->efd, &setter);
#else
    return emodule->evfd.retsno = QP_ERROR;
#endif
    emodule->evfd.errono = errno;
    return emodule->evfd.retsno;
}

qp_int_t
qp_event_reset(qp_event_t *emodule, qp_event_fd_t *eventfd, qp_int_t flag)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_epoll_event_t setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = eventfd->flag | flag;
    emodule->evfd.retsno = epoll_ctl(emodule->evfd.fd, EPOLL_CTL_MOD, 
        eventfd->efd, &setter);
    emodule->evfd.errono = errno;
    return emodule->evfd.retsno;
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_del(qp_event_t *emodule, qp_event_fd_t *eventfd)
{
    if (!emodule || !eventfd) {
        return QP_ERROR;
    }
    
    qp_epoll_event_t setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = 0;
    emodule->evfd.retsno = epoll_ctl(emodule->evfd.fd, EPOLL_CTL_DEL, 
        eventfd->efd, &setter);
    emodule->evfd.errono = errno;
    return emodule->evfd.retsno;
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_accept(qp_event_fd_t* eventfd)
{
    return eventfd ? accept(eventfd->efd, NULL, NULL) : QP_ERROR;
}

qp_int_t
qp_event_close(qp_event_fd_t* eventfd)
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
qp_event_check_close(qp_event_fd_t* eventfd)
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
qp_event_write(qp_event_fd_t* eventfd)
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
qp_event_writev(qp_event_fd_t* eventfd)
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
qp_event_read(qp_event_fd_t* eventfd)
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
qp_event_readv(qp_event_fd_t* eventfd)
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
qp_event_clear_flag(qp_event_fd_t* eventfd)
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
