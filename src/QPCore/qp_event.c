
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
qp_event_is_alloced(qp_event_t* evfd) 
{ return evfd->is_alloced; }

qp_int_t
qp_epoll_create(qp_int_t size);

qp_int_t
qp_event_removeevent(qp_event_t* emodule, qp_event_fd_t* evfd);

qp_int_t
qp_epoll_wait(qp_event_t* emodule, qp_epoll_event_t *events, qp_int_t maxevents, 
    qp_int_t timeout);

qp_int_t
qp_event_add(qp_event_t* evfd, qp_event_fd_t* eventfd);

qp_int_t
qp_event_reset(qp_event_t* evfd, qp_event_fd_t* eventfd, qp_int_t flag);

qp_int_t
qp_event_del(qp_event_t* evfd, qp_event_fd_t* eventfd);

qp_int_t
qp_event_accept(qp_event_fd_t* efd);

qp_int_t
qp_event_close(qp_event_fd_t* efd);

qp_int_t
qp_event_check_close(qp_event_fd_t* efd);

qp_int_t
qp_event_write(qp_event_fd_t* efd);

qp_int_t
qp_event_writev(qp_event_fd_t* efd);

qp_int_t
qp_event_read(qp_event_fd_t* efd);

qp_int_t
qp_event_readv(qp_event_fd_t* efd);

void
qp_event_clear_flag(qp_event_fd_t* fd);


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
        QP_LOGOUT_ERROR("[qp_event_t]Event module create fail.");
        return NULL;
    }
    
    qp_list_init(&emodule->ready);
    qp_list_init(&emodule->listen_ready);
    qp_rbtree_init(&emodule->timer);
    return emodule;
}

qp_event_t*
qp_event_init(qp_event_t* emodule, qp_int_t fd_size, qp_int_t resolution,
    qp_event_opt_handler init, qp_event_opt_handler destroy, 
    bool noblock, bool edge, void* (*idle_cb)(void *), void* idle_arg)
{
    qp_int_t        mod = QP_EPOLL_IN /*| QP_EPOLL_OUT*/ | QP_EPOLL_RDHUP;
    qp_int_t        findex = 0;
    qp_event_fd_t*  eventfd = NULL;
    
    if (1 > fd_size || 1 > resolution) {
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
        mod |= QP_EPOLL_ET /*| QP_EPOLL_ONESHOT */;  /* use for mult thread */
    }

    emodule->idle = idle_cb;
    emodule->idle_arg = idle_arg;
    emodule->init = init;
    emodule->destroy = destroy;
    emodule->event_size = fd_size;
    emodule->timer_resolution = resolution;

    /* create epoll fd */
    emodule->evfd.fd = qp_epoll_create(65535);
    
    if (!qp_fd_is_valid(&emodule->evfd)) {
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
    qp_event_fd_t*    eevent = NULL;
    qp_epoll_event_t* event_queue = NULL;
    struct timeval    etime = {0};
    ssize_t           ret = 0;
    qp_int_t          eevent_num = 0;
    qp_int_t          accept_fd = -1;
    qp_int_t          rflag = 0;
    qp_int_t          uptime;
    qp_int_t          itr = 0;
    
    if (!qp_fd_is_valid(&emodule->evfd)) {
        QP_LOGOUT_ERROR("[qp_event_t]Event Not vaild.");
        return QP_ERROR;
    }

    event_queue = qp_alloc(emodule->event_size * sizeof(qp_epoll_event_t));
    
    if (NULL == event_queue) {
        QP_LOGOUT_ERROR("[qp_event_t]Event list create fail.");
        return QP_ERROR;
    }
    
    emodule->is_run = true;
    gettimeofday(&etime, NULL);
    emodule->timer_begin = (etime.tv_sec << 32) + etime.tv_usec;
    
    /* event loop */
    while (emodule->is_run && qp_pool_used(&emodule->event_pool)) {
        eevent_num = qp_epoll_wait(emodule, event_queue, emodule->event_size,\
            emodule->timer_resolution);

        /* do with error */
        if (1 > eevent_num) {

            if (-1 == eevent_num) {
                
                /* error quit */
                if (EINTR != errno) {
                    QP_LOGOUT_ERROR("[qp_event_t] Epoll wait error.");
                    break;
                }
                
            }

            emodule->timer_begin += emodule->timer_resolution;
            uptime = emodule->timer_resolution;
            QP_LOGOUT_LOG("[qp_event_t]EPOLL WAIT [%s].", strerror(errno));
            /* no error beacuse no event happen */
            if (emodule->idle) {
                emodule->idle(emodule->idle_arg);
            }

            continue;
        }
        
        if (--uptime) {
            gettimeofday(&etime, NULL);
            emodule->timer_begin = (etime.tv_sec << 32) + etime.tv_usec;
        }

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
                        QP_LOGOUT_ERROR("[qp_event_t]Listen fail [%d],[%s].",
                            eevent->efd, strerror(errno));
                        qp_event_close(eevent);
                        qp_event_removeevent(emodule, eevent);
                    }
                    
                    break;
                } 
                
                if (QP_ERROR == \
                    qp_event_addevent(emodule, accept_fd, false, true))
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
    if (qp_fd_is_inited(&emodule->evfd) && !emodule->is_run) { 
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
qp_event_addevent(qp_event_t* emodule, qp_int_t fd, bool listen,bool auto_close)
{
    qp_event_fd_t* revent = NULL;
    
    if (qp_pool_available(&emodule->event_pool) && (fd > QP_FD_INVALID)) {
        /* get the first idle element */
        revent = (qp_event_fd_t*)qp_pool_alloc(&emodule->event_pool, \
            sizeof(qp_event_fd_t));
        revent->efd = fd;

        /* add event to pool */
        if (QP_ERROR == qp_event_add(emodule, revent)) {
            revent->efd = QP_FD_INVALID;
            qp_pool_free(&emodule->event_pool, revent);
            QP_LOGOUT_ERROR("[qp_event_t]Add event fail.");
            return QP_ERROR;
        }
                        
        revent->closed = auto_close;
        revent->listen = listen;
        
        if (revent->listen) {
            revent->stat = QP_EVENT_PROCESS;
            
        } else {
            revent->stat = QP_EVENT_NEW;
        }
        
        QP_LOGOUT_LOG("[qp_event_t]Add event,using connection [%d],current "
            "available [%lu]", revent->index, emodule->event_pool.nfree);
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}

qp_int_t
qp_event_removeevent(qp_event_t* emodule, qp_event_fd_t* evfd)
{
    qp_event_del(emodule, evfd);
    qp_event_clear_flag(evfd);
    qp_pool_free(&emodule->event_pool, evfd);
    QP_LOGOUT_LOG("[qp_event_t]Remove event,current available [%lu]", 
        emodule->event_pool.nfree);
    return QP_SUCCESS;
}

inline void
qp_event_disable(qp_event_t* emodule)
{ emodule->is_run = false; }

qp_int_t
qp_epoll_create(qp_int_t size)
{
#ifdef QP_OS_LINUX
    return epoll_create(size);
#else 
    return QP_ERROR;
#endif
}

qp_int_t
qp_epoll_wait(qp_event_t* emodule, qp_epoll_event_t *events, qp_int_t maxevents, 
    qp_int_t timeout)
{
#ifdef QP_OS_LINUX
    return epoll_wait(emodule->evfd.fd, events, maxevents, timeout);
#else 
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_add(qp_event_t *evfd, qp_event_fd_t *eventfd)
{
    qp_epoll_event_t setter;
    
    if (eventfd->noblock) {
        fcntl(eventfd->efd, F_SETFL, fcntl(eventfd->efd, F_GETFL) | O_NONBLOCK);
    }
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = eventfd->flag;
    return epoll_ctl(evfd->evfd.fd, EPOLL_CTL_ADD, eventfd->efd, &setter);
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_reset(qp_event_t *evfd, qp_event_fd_t *eventfd, qp_int_t flag)
{
    qp_epoll_event_t setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = eventfd->flag | flag;
    return epoll_ctl(evfd->evfd.fd, EPOLL_CTL_MOD, eventfd->efd, &setter);
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_del(qp_event_t *evfd, qp_event_fd_t *eventfd)
{
    qp_epoll_event_t setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = eventfd;
    setter.events = 0;
    return epoll_ctl(evfd->evfd.fd, EPOLL_CTL_DEL, eventfd->efd, &setter);
#else
    return QP_ERROR;
#endif
}

qp_int_t
qp_event_accept(qp_event_fd_t* efd)
{
    return accept(efd->efd, NULL, NULL);
}

qp_int_t
qp_event_close(qp_event_fd_t* efd)
{
    if (efd->closed && (QP_FD_INVALID != efd->efd)) {
        close(efd->efd);
    }

    efd->efd = QP_FD_INVALID;
    return QP_SUCCESS;
}


qp_int_t
qp_event_check_close(qp_event_fd_t* efd)
{
    if (efd->nativeclose) {
        qp_event_close(efd);
        return QP_ERROR;
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_write(qp_event_fd_t* efd)
{
    size_t rest;
    ssize_t ret;
    
    if (efd->write && (QP_FD_INVALID != efd->efd)) {

        if (efd->write_done < efd->field.writebuf_max) {
            rest = efd->field.writebuf_max - efd->write_done;
            ret = write(efd->efd, efd->field.writebuf.block + efd->write_done, 
                rest);

            if (1 > ret) {
                efd->write = 0;
                
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    /* need close */
                    qp_event_close(efd);
                    efd->write_finish = 1;
                    return QP_ERROR;
                }

                /* write buf full and write event hup */
                efd->writehup = 1;
                
            } else {
                efd->write_done = efd->write_done + ret;
            }

        } else {
            /* write finish */
            efd->write = 0;
            efd->write_finish = 1;
        }
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_writev(qp_event_fd_t* efd)
{
    ssize_t ret;

    if (efd->write && efd->field.writebuf_max && (QP_FD_INVALID != efd->efd)) {
        ret = writev(efd->efd, efd->field.writebuf.vector, \
            efd->field.writebuf_max);
        efd->write = 0;
        efd->write_finish = 1;

        if (1 > ret) {

            if ((0 == ret) 
                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
            {
                qp_event_close(efd);
                return QP_ERROR;
            }
        }
    }
    
    return QP_SUCCESS;
}

qp_int_t
qp_event_read(qp_event_fd_t* efd)
{
    size_t rest;
    ssize_t ret;
    
    if (efd->field.read_atleast > efd->field.readbuf_max) {
        efd->field.read_atleast = efd->field.readbuf_max;
    }

    if (efd->read && (QP_FD_INVALID != efd->efd)) {

        if (efd->read_done < efd->field.readbuf_max) {
            rest = efd->field.readbuf_max - efd->read_done;
            ret = read(efd->efd, efd->field.readbuf.block+efd->read_done,rest);

            if (1 > ret) {
                efd->read = 0;

                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    qp_event_close(efd);
                    efd->read_finish = 1;
                    return QP_ERROR;
                }
                
                if (efd->field.read_atleast) {
                    efd->read_finish = \
                        efd->field.read_atleast > efd->read_done ? 0 : 1;
                    
                } else {
                    efd->read_finish = 1;
                }

            } else {
                efd->read_done = efd->read_done + ret;
            }

        } else {
            efd->read = 0;
            efd->read_finish = 1;

            if (efd->edge) {
                /* we will rest beacues kernel buf still have data */
                efd->readhup = 1;
            }
        }
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_readv(qp_event_fd_t* efd)
{
    ssize_t ret;

    if (efd->read && efd->field.readbuf_max && (QP_FD_INVALID != efd->efd)) {
        ret = readv(efd->efd, efd->field.readbuf.vector, efd->field.readbuf_max);
        efd->read = 0;
        efd->read_finish = 1;

        if (1 > ret) {

            if ((0 == ret) 
                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
            {
                qp_event_close(efd);
                return QP_ERROR;
            }
        }
    }
    

    return QP_SUCCESS;
}

void
qp_event_clear_flag(qp_event_fd_t* fd)
{
    fd->listen = 0;
    fd->closed = 0;
    fd->stat = QP_EVENT_IDL;
    fd->nativeclose = 0;
    fd->peerclose = 0;
    fd->write = 0;
    fd->writehup = 0;
    fd->write_finish = 0;
    fd->read = 0;
    fd->readhup = 0;
    fd->read_finish = 0;
    fd->read_done = 0;
    fd->write_done = 0;
}
