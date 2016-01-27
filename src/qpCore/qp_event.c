
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
qp_event_init(qp_event_t *emodule, 
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
    void*        (*wait_cb)(void *),
    void*        wait_arg)
{
    (NULL != fds) ? \
        (listen_fds = NULL, listen_fd_size = 0) :
        ((0 == listen_fd_size || NULL == listen_fds) ? (fd_size = 0) : 1);

    if (1 > fd_size) {
        return NULL;
    }
    
    emodule = qp_event_create(emodule);
    
    if (NULL == emodule) {
        return NULL;
    }

#ifndef QP_OS_LINUX
    return NULL;
#endif

    qp_int_t        mod = EPOLLIN /*| EPOLLOUT*/ | flag;
    qp_uint32_t     findex = 0;
    qp_event_fd_t*  eventfd = NULL;
    
    if (NULL == fds) {
        mod |= EPOLLRDHUP;
    }

    if (edge) {
        mod |= EPOLLET /*| EPOLLONESHOT */;  /* use for mult thread */
    }

    emodule->available = 0;
    emodule->event_size = fd_size;
    emodule->event_listen_size = listen_fd_size;
    emodule->event_cb = wait_cb;
    emodule->event_cb_arg = wait_arg;
    emodule->event_fd_init_handler = qp_event_fd_init_handler;
    emodule->event_fd_destory_handler = qp_event_fd_destroy_handler;
    qp_list_init(&(emodule->ready));

    /* create epoll fd */
    emodule->evfd.fd = epoll_create(65536);
    
    if (!qp_fd_is_valid(&(emodule->evfd))) {
        qp_event_destroy(emodule);
        return NULL;
    }
    
    /* init event pool */
    if (NULL == qp_pool_init(&(emodule->event_pool), sizeof(qp_event_fd_t), \
        emodule->event_size + emodule->event_listen_size))
    {
        qp_event_destroy(emodule);
        return NULL;
    }

    /* init poollist */
    
    
    for (findex = emodule->event_size + emodule->event_listen_size; \
        findex; findex--) 
    {   
        eventfd = \
            (qp_event_fd_t*)qp_pool_to_array(&(emodule->event_pool), findex - 1);
        eventfd->index = findex - 1;
        eventfd->efd = -1;
        eventfd->flag = mod;
        eventfd->listen = 0;
        eventfd->closed = 1;
        eventfd->noblock = noblock;
        eventfd->edge = edge;
        eventfd->nativeclose = 0;
        eventfd->peerclose = 0;
        eventfd->writehup = 0;
        eventfd->write = 0;
        eventfd->readhup = 0;
        eventfd->read = 0;
        eventfd->do_myself = NULL;

        memset(&(eventfd->field), 0, sizeof(qp_event_data_t));
        eventfd->field.readbuf = emodule->combuf;
        eventfd->field.read_max = QP_EVENT_COMMONDATA_SIZE;
        eventfd->field.writebuf = emodule->combuf;
        eventfd->field.write_max = QP_EVENT_COMMONDATA_SIZE;
        qp_list_init(&(eventfd->ready_next));
        
        if (emodule->event_fd_init_handler) {
            emodule->event_fd_init_handler(&(eventfd->field), \
                &(eventfd->do_myself),
                eventfd->efd);
        }
    }
    
    /* allocate listen fd list */
    for (findex = 0; findex < emodule->event_listen_size; findex++) {
        
        if (listen_fds[findex].fd < 1) {
            continue;
        }
        
        eventfd = (qp_event_fd_t*)qp_pool_alloc(&(emodule->event_pool), \
            sizeof(qp_event_fd_t));
        
        if (!eventfd) {
            qp_event_destroy(emodule);
            return NULL;
        }
        
        eventfd->efd = listen_fds[findex].fd;
        eventfd->listen = 1;
        eventfd->closed = 0;
        eventfd->do_myself = NULL;
        
        if (QP_EVENT_ERROR == qp_event_add(emodule, eventfd)) {
            qp_pool_free(&(emodule->event_pool), eventfd);
            continue;
        }
        
        emodule->available++;
    }
    
    if (!fds)  {
        return emodule;
    }
    
    /* allocate fd list */
    for (findex = 0; findex < emodule->event_size; findex++) {
        
        if (fds[findex].fd < 1) {
            continue;
        }
        
        eventfd = (qp_event_fd_t*)qp_pool_alloc(&(emodule->event_pool), \
            sizeof(qp_event_fd_t));
        
        if (!eventfd) {
            qp_event_destroy(emodule);
            return NULL;
        }
        
        eventfd->efd = fds[findex].fd;
        eventfd->closed = 0;
        
        if (QP_EVENT_ERROR == qp_event_add(emodule, eventfd)) {
            qp_pool_free(&(emodule->event_pool), eventfd);
            continue;
        }

        emodule->available++;
    }

    return emodule;
}


qp_int_t
qp_event_tiktok(qp_event_t *emodule, int queue, int *runstat)
{
    if (!qp_fd_is_valid(&(emodule->evfd))) {
        return QP_EVENT_ERROR;
    }

    qp_int_t          revent_num = 0;
    qp_uint32_t       rflag = 0;
    qp_int_t          (*readhandler)(qp_event_t*, qp_event_fd_t*);
    qp_int_t          (*writehandler)(qp_event_t*, qp_event_fd_t*);
    qp_event_fd_t*    revent = NULL;
    qp_event_fd_t*    eevent = NULL;
    epoll_event_t*    event_queue = NULL;
    ssize_t           ret = 0;
    qp_int_t          timeout = (NULL == emodule->event_cb) ? -1 : 0;
    qp_int_t          acceptfd = -1;
    qp_int_t          itr = 0;

    event_queue = qp_alloc(queue * sizeof(epoll_event_t));
    
    if (NULL == event_queue) {
        QP_LOGOUT_ERROR("[qp_event_t]Event list create fail.");
        return QP_EVENT_ERROR;
    }

    while (*runstat && emodule->available) {
        revent_num = \
            epoll_wait(emodule->evfd.fd, event_queue, queue, timeout);

        /* do with error */
        if (0 > revent_num) {

            if (EINTR != errno) {
                *runstat = 0;
                QP_LOGOUT_ERROR("[qp_event_t] Epollwait error.");
                break;
            }

            if (emodule->event_cb) {
                emodule->event_cb(emodule->event_cb_arg);
            }

            continue;
        }

        for (itr = 0; itr < revent_num; itr++) {

            if (NULL == (revent = (qp_event_fd_t*)(event_queue[itr].data.ptr))){
                continue;
            }

            rflag =  event_queue[itr].events;

            /* listen event first */
            if (revent->listen) {

                if (rflag & EPOLLERR) {
                    qp_event_close(emodule, revent);
                    emodule->available--;
                    qp_pool_free(&(emodule->event_pool), revent);
                    QP_LOGOUT_ERROR("[qp_event_t] Listen event error.");
                    continue;
                }

                do {
                    acceptfd = accept(revent->efd, NULL, NULL);

                    if (acceptfd > 0) {

                        if (qp_pool_available(&(emodule->event_pool))) {
                            /* get the first idle element */
                            eevent = \
                                (qp_event_fd_t*)qp_pool_alloc(\
                                &(emodule->event_pool),sizeof(qp_event_fd_t));
                            
                            eevent->efd = acceptfd;

                            /* add event to pool */
                            if (QP_EVENT_ERROR != qp_event_add(emodule, eevent)){
                                emodule->available++;
                                
                            } else {
                                qp_pool_free(&(emodule->event_pool), eevent);
                                close(acceptfd);
                                QP_LOGOUT_ERROR("[qp_event_t]Add event fail.");
                                continue;
                            }

                            QP_LOGOUT_LOG("[qp_event_t]Using connection [%d]", \
                                eevent->index);
                            QP_LOGOUT_LOG("[qp_event_t]Current available : "
                                "[%d].", emodule->available);

                        } else {
                            /* connection pool used up */
                            close(acceptfd);
                            QP_LOGOUT_LOG("[qp_event_t]Connection used up");
                        }

                    } else {
                        /* accept error */

                        if (!((EAGAIN == errno)
                            || (ECONNABORTED == errno)
                            || (EPROTO == errno)
                            || (EINTR == errno)))
                        {
                            QP_LOGOUT_ERROR("[qp_event]Listen on [%d] "\
                                "error:[%d][%s]",\
                                revent->efd, errno, strerror(errno));
                            qp_event_close(emodule, revent);
                            emodule->available--;
                            qp_pool_free(&(emodule->event_pool), revent);
                        }

                        break;
                    }

                } while (revent->edge);

            } else {
                revent->nativeclose |= (EPOLLHUP | EPOLLERR) & rflag;
                revent->peerclose |= EPOLLRDHUP & rflag;
                revent->writehup = revent->peerclose;
                revent->read = EPOLLIN & rflag;

                /* if no event */
                if (!(revent->read | revent->write | revent->nativeclose)) {
                    continue;
                }

                /* add read/write event to ready list */
                qp_list_push(&(emodule->ready), &(revent->ready_next));
            }
        }


        /* do read/write event */
        while (!qp_list_is_empty(&(emodule->ready))) {
            eevent = qp_list_data(qp_list_first(&(emodule->ready)), \
                qp_event_fd_t, ready_next);
            qp_list_pop(&(emodule->ready));
            
            writehandler = (NULL ==  eevent->field.writeiovec) ? \
                    qp_event_write : qp_event_writev;
            readhandler = (NULL ==  eevent->field.readiovec) ? \
                    qp_event_read : qp_event_readv;

            do {
                /* connection closed */
                if (QP_EVENT_ERROR == qp_event_check_close(emodule, eevent)) {
                    break;
                }

                /* if all events have done */
                if (!(eevent->read | eevent->write)) {
                    break;
                }

                /* do with read/write event */
                writehandler(emodule, eevent);
                readhandler(emodule, eevent);

                /* if event not available quit */
                if (1 > eevent->efd) {
                    break;
                }

            } while (eevent->edge && !(eevent->readhup && eevent->writehup));

            eevent->readhup = 0;
            eevent->writehup = 0;

            if (eevent->do_myself) {
                ret = eevent->do_myself(eevent->efd, &(eevent->field));

                if (QP_EVENT_SUCCESS > ret) {
                    /* error close */
                    if (0 < eevent->efd) {
                        qp_event_close(emodule, eevent);
                    }

                } else {

                    /* need write data */
                    if (ret & EPOLLOUT) {
                        eevent->write = 1;
                            
                        if (eevent->edge) {
                            qp_event_reset(emodule, eevent, EPOLLOUT);
                        }

                    } else {

                        /* need close */
                        if ((ret & EPOLLHUP) && (0 < eevent->efd)) {
                            qp_event_close(emodule, eevent);
                        }
                    }
                }

            } else {
                /* if no user callback, always set read_start to 0 */
                eevent->field.read_start = 0;
            }
            
            if (eevent->efd < 1) {
                eevent->field.read_start = 0;
                eevent->field.read_offset = 0;
                eevent->field.write_offset = 0;
                eevent->field.write_start = 0;
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
    return QP_EVENT_SUCCESS;
}


qp_int_t
qp_event_destroy(qp_event_t *emodule)
{
    if (qp_fd_is_inited(&(emodule->evfd))) { 
        size_t i = emodule->event_size + emodule->event_listen_size;
        qp_event_fd_t* eventfd = NULL;
        
        for (; i; i--) {
            eventfd = (qp_event_fd_t*)qp_pool_to_array(&(emodule->event_pool), \
                i - 1);
            if (eventfd->closed && (0 < eventfd->efd)) {
                close(eventfd->efd);
            }

            if (emodule->event_fd_destory_handler) {
                emodule->event_fd_destory_handler(&(eventfd->field));
            }
        }

        qp_pool_destroy(&(emodule->event_pool), true);
        qp_fd_destroy(&(emodule->evfd));
        
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
    if (eventfd->noblock) {
        fcntl(eventfd->efd, F_SETFL, fcntl(eventfd->efd, F_GETFL) | O_NONBLOCK);
    }
#ifdef  QP_OS_LINUX
    evfd->setter.data.ptr = eventfd;
    evfd->setter.events = eventfd->flag;
    return epoll_ctl(evfd->evfd.fd, EPOLL_CTL_ADD, \
        eventfd->efd, &(evfd->setter));
#else
    return QP_EVENT_ERROR;
#endif
}

qp_int_t
qp_event_reset(qp_event_t *evfd, qp_event_fd_t *eventfd, qp_int_t flag)
{
#ifdef  QP_OS_LINUX
    evfd->setter.data.ptr = eventfd;
    evfd->setter.events = eventfd->flag | flag;
    return epoll_ctl(evfd->evfd.fd, EPOLL_CTL_MOD, \
        eventfd->efd, &(evfd->setter));
#else
    return QP_EVENT_ERROR;
#endif
}


qp_int_t
qp_event_del(qp_event_t *evfd, qp_event_fd_t *eventfd)
{
#ifdef  QP_OS_LINUX
    evfd->setter.data.ptr = eventfd;
    evfd->setter.events = 0;
    return epoll_ctl(evfd->evfd.fd, EPOLL_CTL_DEL, \
        eventfd->efd, &(evfd->setter));
#else
    return QP_EVENT_ERROR;
#endif
}

void
qp_event_close(qp_event_t* module, qp_event_fd_t* efd)
{
    qp_event_del(module, efd);

    if (efd->closed) {
        close(efd->efd);
    }

    efd->efd = QP_FD_INVALID;
}


qp_int_t
qp_event_check_close(qp_event_t* module, qp_event_fd_t* efd)
{
    if (efd->nativeclose) {
        qp_event_close(module, efd);
        return QP_EVENT_ERROR;
    }

    return QP_EVENT_SUCCESS;
}

qp_int_t
qp_event_write(qp_event_t* module, qp_event_fd_t* efd)
{
    size_t rest;
    ssize_t ret;

    if (efd->write && !efd->writehup && (0 < efd->efd)) {

        if ((efd->field.write_start < efd->field.write_offset)
            && (efd->field.write_offset <= efd->field.write_max))
        {
            rest = efd->field.write_offset - efd->field.write_start;
            ret = write(efd->efd, efd->field.writebuf + efd->field.write_start,\
                rest);

            if (1 > ret) {

                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    /* need close */
                    qp_event_close(module, efd);
                    return QP_EVENT_ERROR;
                }

                /*
                 * we do not reset event flag beacuse when kernel buffer
                 * available again, write event will happen itself.(if EPOLLOUT was
                 * set in init function.)
                */
                efd->writehup = 1;
                qp_event_reset(module, efd, EPOLLOUT);

            } else {
                efd->field.write_start = efd->field.write_start + ret;
                efd->write = (efd->field.write_start < efd->field.write_offset)? 
                    1 : 0;
            }

        } else {
            efd->write = 0;
            /*we do not reset event flag also */
        }
    }

    return QP_EVENT_SUCCESS;
}


qp_int_t
qp_event_read(qp_event_t* module, qp_event_fd_t* efd)
{
    size_t rest;
    ssize_t ret;

    if (efd->read && !efd->readhup && (0 < efd->efd)) {

        if (efd->field.read_start < efd->field.read_max) {
            rest = efd->field.read_max - efd->field.read_start;
            ret = read(efd->efd, efd->field.readbuf + efd->field.read_start, \
                rest);

            if (1 > ret) {

                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno 
                    || EINTR == errno))
                {
                    qp_event_close(module, efd);
                    return QP_EVENT_ERROR;
                }

                efd->readhup = 1;
                efd->read = 0;

            } else {
                efd->field.read_offset = efd->field.read_offset + ret;
                efd->field.read_start = efd->field.read_offset;

                /* if user buf used up but kernel buf still have data */
                if (efd->field.read_offset >= efd->field.read_max) {
                    qp_event_reset(module, efd, 0);
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

    return QP_EVENT_SUCCESS;
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
                return QP_EVENT_ERROR;
            }
        }
    }

    return QP_EVENT_SUCCESS;
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
                return QP_EVENT_ERROR;
            }
        }
    }

    return QP_EVENT_SUCCESS;
}
