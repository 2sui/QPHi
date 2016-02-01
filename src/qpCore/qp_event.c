
/**
  * Copyright (C) sui
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


void
qp_event_close(qp_event_t* module, qp_event_fd_t* efd);

qp_int_t
qp_event_check_close(qp_event_t* module, qp_event_fd_t* efd);

qp_int_t
qp_event_write(qp_event_t* module, qp_event_fd_t* efd);

qp_int_t
qp_event_read(qp_event_t* module, qp_event_fd_t* efd);

qp_int_t
qp_event_readv(qp_event_t* module, qp_event_fd_t* efd);

qp_int_t
qp_event_writev(qp_event_t* module, qp_event_fd_t* efd);



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
    
    if (NULL == qp_fd_init(&(emodule->evfd), QP_FD_TYPE_EVENT, false)) {
        qp_event_is_alloced(emodule) ? qp_free(emodule) : 1;
        QP_LOGOUT_ERROR("[qp_event_t]Event module create fail.");
        return NULL;
    }
    
    return emodule;
}

qp_event_t*
qp_event_init(qp_event_t* emodule, qp_uint32_t fd_size, 
    qp_int_t (*qp_event_fd_init_handler)(qp_event_data_t*),
    qp_int_t (*qp_event_fd_destroy_handler)(qp_event_data_t*),
    bool noblock, bool edge, void* (*idle_cb)(void *), void* idle_arg)
{
    qp_int_t        mod = EPOLLIN /*| EPOLLOUT*/ | EPOLLRDHUP;
    qp_uint32_t     findex = 0;
    qp_event_fd_t*  eventfd = NULL;
    
    if (0 == fd_size) {
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
        mod |= EPOLLET /*| EPOLLONESHOT */;  /* use for mult thread */
    }

    emodule->available = 0;
    emodule->event_size = 0;
    emodule->event_idle_cb = idle_cb;
    emodule->event_idle_cb_arg = idle_arg;
    emodule->event_fd_init_handler = qp_event_fd_init_handler;
    emodule->event_fd_destory_handler = qp_event_fd_destroy_handler;
    qp_list_init(&emodule->ready);
    qp_list_init(&emodule->listen_ready);

    /* create epoll fd */
    emodule->evfd.fd = qp_epoll_create(65535);
    
    if (!qp_fd_is_valid(&(emodule->evfd))) {
        qp_event_destroy(emodule);
        return NULL;
    }
    
    /* init event pool */
    if (NULL == qp_pool_init(&(emodule->event_pool), sizeof(qp_event_fd_t), \
        emodule->event_size))
    {
        qp_event_destroy(emodule);
        return NULL;
    }
    
    emodule->event_size = fd_size;

    /* init poollist */
    for (; findex < emodule->event_size; findex++) {   
        eventfd = \
            (qp_event_fd_t*)qp_pool_to_array(&(emodule->event_pool), findex);
        eventfd->index = findex;
        eventfd->efd = QP_FD_INVALID;
        eventfd->flag = mod;
        eventfd->noblock = noblock;
        eventfd->edge = edge;
        
        eventfd->listen = 0;
        eventfd->closed = 0;
        
        eventfd->nativeclose = 0;
        eventfd->peerclose = 0;
        eventfd->write = 0;
        eventfd->read = 0;
        eventfd->stat = QP_EVENT_IDL;
        
        qp_list_init(&(eventfd->ready_next));
        memset(&(eventfd->field), 0, sizeof(qp_event_data_t));
    }
    
    return emodule;
}

qp_int_t
qp_event_tiktok(qp_event_t *emodule, qp_int_t *runstat)
{
    qp_uint32_t       rflag = 0;
    qp_int_t          revent_num = 0;
    qp_int_t          (*readhandler)(qp_event_t*, qp_event_fd_t*);
    qp_int_t          (*writehandler)(qp_event_t*, qp_event_fd_t*);
    qp_event_fd_t*    revent = NULL;
    qp_event_fd_t*    eevent = NULL;
    qp_epoll_event_t* event_queue = NULL;
    ssize_t           ret = 0;
    qp_int_t          timeout = (NULL == emodule->event_idle_cb) ? -1 : 0;
    qp_int_t          acceptfd = -1;
    qp_int_t          itr = 0;
    
    if (!qp_fd_is_valid(&(emodule->evfd))) {
        return QP_ERROR;
    }

    event_queue = qp_alloc(emodule->event_size * sizeof(qp_epoll_event_t));
    
    if (NULL == event_queue) {
        QP_LOGOUT_ERROR("[qp_event_t]Event list create fail.");
        return QP_ERROR;
    }

    while (*runstat && emodule->available) {
        revent_num = \
            qp_epoll_wait(emodule->evfd.fd, event_queue, emodule->event_size, \
            timeout);

        /* do with error */
        if (0 > revent_num) {

            if (EINTR != errno) {
                *runstat = 0;
                QP_LOGOUT_ERROR("[qp_event_t] Epollwait error.");
                break;
            }

            if (emodule->event_idle_cb) {
                emodule->event_idle_cb(emodule->event_idle_cb_arg);
            }

            continue;
        }

        for (itr = 0; itr < revent_num; itr++) {
            revent = (qp_event_fd_t*)(event_queue[itr].data.ptr);
            rflag =  event_queue[itr].events;
            
            revent->nativeclose |= (EPOLLHUP | EPOLLERR) & rflag;
            revent->peerclose |= EPOLLRDHUP & rflag;
            revent->write = EPOLLOUT & rflag & !revent->peerclose;
            revent->read = EPOLLIN & rflag;

            /* if no event */
            if (!(revent->read | revent->write | revent->nativeclose)) {
                continue;
            }

            /* add read/write event to ready list */
            if (revent->listen) {
                qp_list_push(&emodule->listen_ready, &revent->ready_next);
                
            } else {
                qp_list_push(&emodule->ready, &revent->ready_next);
            }
            
            continue;
        }
        
        /* listen event */
        while (!qp_list_is_empty(&(emodule->listen_ready))) {
            eevent = qp_list_data(qp_list_first(&emodule->listen_ready), \
                qp_event_fd_t, ready_next);
            qp_list_pop(&emodule->listen_ready);
            
            if (eevent->nativeclose) {
                QP_LOGOUT_ERROR("[qp_event_t] Listen event error at [%d].", 
                    eevent->index);
                qp_event_close(emodule, eevent);
                qp_event_del(emodule, eevent);
                qp_pool_free(&(emodule->event_pool), eevent);
                qp_atom_sub(&emodule->available);
                continue;
            }
            
            do {
                acceptfd = accept(eevent->efd, NULL, NULL);

                if (QP_FD_INVALID != acceptfd) {
                    
                    if (qp_pool_available(&(emodule->event_pool))) {
                        /* get the first idle element */
                        revent = (qp_event_fd_t*)qp_pool_alloc(\
                            &(emodule->event_pool), sizeof(qp_event_fd_t));
                            
                        revent->efd = acceptfd;

                        /* add event to pool */
                        if (QP_ERROR != qp_event_add(emodule, revent)){
                            qp_atom_add(&emodule->available);
                            revent->stat = QP_EVENT_NEW;
                                
                        } else {
                            qp_pool_free(&emodule->event_pool, revent);
                            close(acceptfd);
                            QP_LOGOUT_ERROR("[qp_event_t]Add event fail.");
                            continue;
                        }

                        QP_LOGOUT_LOG("[qp_event_t]Using connection [%d]", \
                            revent->index);
                        QP_LOGOUT_LOG("[qp_event_t]Current available : "
                            "[%lu].", emodule->available);

                    } else {
                        /* connection pool used up */
                        close(acceptfd);
                        QP_LOGOUT_LOG("[qp_event_t]Connection used up");
                    }

                } else {
                    
                    /* accept error */
                    if (!((EAGAIN == errno) || (ECONNABORTED == errno)
                        || (EPROTO == errno) || (EINTR == errno)))
                    {
                        QP_LOGOUT_ERROR("[qp_event]Listen on [%d] error:[%d][%s]",\
                            eevent->efd, errno, strerror(errno));
                        qp_event_close(emodule, eevent);
                        qp_event_del(emodule, eevent);
                        qp_pool_free(&(emodule->event_pool), eevent);
                        qp_atom_sub(&emodule->available);
                    }

                    break;
                }

            } while (revent->edge);
                
        }
            
        /* do read/write event */
        while (!qp_list_is_empty(&(emodule->ready))) {
            eevent = qp_list_data(qp_list_first(&(emodule->ready)), \
                qp_event_fd_t, ready_next);
            qp_list_pop(&(emodule->ready));
            
            writehandler = (QP_EVENT_BLOCK_OPT == eevent->field.next_write_opt)?\
                qp_event_write : qp_event_writev;
            readhandler = (QP_EVENT_BLOCK_OPT == eevent->field.next_read_opt) ? \
                qp_event_read : qp_event_readv;

            /* do with write/read events */
            do {
                /* connection closed */
                if (QP_ERROR == qp_event_check_close(emodule, eevent)) {
                    break;
                }
                
                /* if all events have done */
                if (!(eevent->read | eevent->write)) {
                    break;
                }
                
                /* if no buf is assigned */
                if (!eevent->field.writebuf.block) {
                    eevent->field.writebuf.block = emodule->combuf;
                    eevent->field.writebuf_max = QP_EVENT_COMMONDATA_SIZE;
                }
                
                if (!eevent->field.readbuf.block) {
                    eevent->field.readbuf.block = emodule->combuf;
                    eevent->field.readbuf_max = QP_EVENT_COMMONDATA_SIZE;
                }
                
                /* do with read/write event */
                if (QP_ERROR == writehandler(emodule, eevent) 
                    || QP_ERROR == readhandler(emodule, eevent))
                {
                    break;
                }
                
                /* recover buf */
                if (eevent->field.readbuf.block == emodule->combuf) {
                    eevent->field.readbuf.block = NULL;
                    eevent->field.readbuf_max = 0;
                }
                
                if (eevent->field.writebuf.block == emodule->combuf) {
                    eevent->field.writebuf.block = NULL;
                    eevent->field.writebuf_max = 0;
                }
                
            } while (eevent->edge);

            /* process the events */
            if (eevent->field.process_handler && eevent->process) {
                
                if (QP_FD_INVALID == eevent->efd) {
                    eevent->stat = QP_EVENT_CLOSE;    
                }
                
                ret = eevent->field.process_handler(&eevent->field, eevent->efd,
                    eevent->stat, eevent->read_done, eevent->write_done);
                
                if (QP_EVENT_NEW == eevent->stat) {
                    eevent->stat = QP_EVENT_PROCESS;
                }
                
                if (QP_FD_INVALID != eevent->efd) {
                    
                    if (QP_SUCCESS > ret) {
                        qp_event_close(emodule, eevent);
                        
                    } else {
                        
                        if (ret & EPOLLOUT) {
                            qp_event_reset(emodule, eevent, EPOLLOUT);
                            eevent->write_done = 0;
                        }
                    }
                }
                
                eevent->process = 0;
                eevent->read_done = 0;
            }
            
            if (QP_FD_INVALID == eevent->efd) {
                
                eevent->nativeclose = 0;
                eevent->peerclose = 0;
                eevent->write = 0;
                eevent->read = 0;
                qp_pool_free(&(emodule->event_pool), eevent);
                emodule->available--;
                QP_LOGOUT_LOG("[qp_event]Current available : [%d].", \
                    emodule->available);
            }
            
        }  // while
    }  // while

    qp_free(event_queue);
    return QP_SUCCESS;
}


qp_int_t
qp_event_destroy(qp_event_t *emodule)
{
    if (qp_fd_is_inited(&emodule->evfd)) { 
        size_t i = emodule->event_size;
        qp_event_fd_t* eventfd = NULL;
        
        for (; i; i--) {
            eventfd = (qp_event_fd_t*)qp_pool_to_array(&emodule->event_pool, \
                i - 1);
            if (eventfd->closed && (QP_FD_INVALID != eventfd->efd)) {
                qp_event_close(emodule, eventfd);
            }

            if (emodule->event_fd_destory_handler) {
                emodule->event_fd_destory_handler(&eventfd->field);
            }
        }

        qp_pool_destroy(&emodule->event_pool, true);
        qp_fd_destroy(&emodule->evfd);
        
        if (qp_event_is_alloced(emodule)) {
            qp_free(emodule);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
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

void
qp_event_close(qp_event_t* module, qp_event_fd_t* efd)
{
    if (efd->closed && (QP_FD_INVALID == efd->efd)) {
        close(efd->efd);
        efd->efd = QP_FD_INVALID;
    }

}


qp_int_t
qp_event_check_close(qp_event_t* module, qp_event_fd_t* efd)
{
    if (efd->nativeclose) {
        qp_event_close(module, efd);
        return QP_ERROR;
    }

    return QP_SUCCESS;
}

qp_int_t
qp_event_write(qp_event_t* module, qp_event_fd_t* efd)
{
    size_t rest;
    ssize_t ret;
    
    /* write_atleast is always equal to write */
//    if (efd->field.write_atleast > efd->field.writebuf_max) {
//        efd->field.write_atleast = efd->field.writebuf_max;
//    }

    if (efd->write/* && !efd->writehup*/ && (QP_FD_INVALID != efd->efd)) {

        if (efd->field.write_done < efd->field.writebuf_max) {
            rest = efd->field.writebuf_max - efd->field.write_done;
            ret = write(efd->efd, efd->field.writebuf.block + \
                efd->field.write_done, rest);

            if (1 > ret) {
                efd->write = 0;
                
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    /* need close */
                    qp_event_close(module, efd);
                    efd->process |= 1;
                    return QP_ERROR;
                }

                /* write buf in kernel is full */
                if (efd->field.write_done < efd->field.writebuf_max) {
                    qp_event_reset(module, efd, EPOLLOUT);
                }
                
            } else {
                efd->field.write_done = efd->field.write_done + ret;
                efd->write = (efd->field.write_done < efd->field.writebuf_max)? 
                    1 : 0;
            }

        } else {
            efd->write = 0;
        }
    }

    return QP_SUCCESS;
}


qp_int_t
qp_event_read(qp_event_t* module, qp_event_fd_t* efd)
{
    size_t rest;
    ssize_t ret;
    
    if (efd->field.read_atleast > efd->field.readbuf_max) {
        efd->field.read_atleast = efd->field.readbuf_max;
    }

    if (efd->read/* && !efd->readhup*/ && (QP_FD_INVALID != efd->efd)) {

        if (efd->field.read_done < efd->field.readbuf_max) {
            rest = efd->field.readbuf_max - efd->field.read_done;
            ret = read(efd->efd, efd->field.readbuf.block + efd->field.read_done,\
                rest);

            if (1 > ret) {

                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    qp_event_close(module, efd);
                    efd->read = 0;
                    efd->process = 1;
                    return QP_ERROR;
                }

                efd->read = 0;
                
                if (efd->field.read_atleast) {
                    efd->process = \
                        efd->field.read_atleast == efd->field.read_done ?
                        1 : 0;
                    
                } else {
                    efd->process = 1;
                }

            } else {
                efd->field.read_done = efd->field.read_done + ret;

                /* if user buf used up but kernel buf still have data */
                if (efd->field.read_done >= efd->field.readbuf_max) {
                    qp_event_reset(module, efd, 0);
                    efd->read = 0;

                    if (efd->edge) {
                    /* we will rest beacues kernel buf still have data */
                        qp_event_reset(module, efd, 0);
                    }
                    
                    efd->process = 1;
                }
            }

        } else {
            efd->read = 0;

            if (efd->edge) {
                /* we will rest beacues kernel buf still have data */
                qp_event_reset(module, efd, 0);
            }
        }
    }

    return QP_SUCCESS;
}


qp_int_t
qp_event_readv(qp_event_t* module, qp_event_fd_t* efd)
{
    ssize_t ret;

    if (efd->read && !efd->readhup 
        && (efd->field.readiovec_size > 0)&& (0 < efd->efd)) 
    {
        ret = readv(efd->efd, efd->field.readiovec, efd->field.readiovec_size);
        efd->readhup = 1;
        efd->read = 0;

        if (1 > ret) {

            if ((0 == ret) 
                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
            {
                qp_event_close(module, efd);
                return QP_ERROR;
            }
        }
    }

    return QP_SUCCESS;
}


qp_int_t
qp_event_writev(qp_event_t* module, qp_event_fd_t* efd)
{
    ssize_t ret;

    if (efd->write && !efd->writehup && (efd->field.writeiovec_size > 0) 
        && (0 < efd->efd)) 
    {
        ret = writev(efd->efd,efd->field.writeiovec,efd->field.writeiovec_size);
        efd->writehup = 1;
        efd->write = 0;

        if (1 > ret) {

            if ((0 == ret) 
                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
            {
                qp_event_close(module, efd);
                return QP_ERROR;
            }
        }
    }

    return QP_SUCCESS;
}
