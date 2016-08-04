
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

struct qp_event_source_s {
    struct qp_list_s           ready_next;     /* event source list */
    struct qp_rbtree_node_s    timer_node;     
    qp_uchar_t*                cache;          /* read/write cache */        
    qp_int_t                   index;
    qp_int_t                   source_fd;
    qp_int_t                   events;
    qp_int_t                   revents;
    qp_int_t                   cache_size;
    qp_int_t                   cache_offset;
    
    qp_uint32_t                noblock     :1;     /* need noblock */
    qp_uint32_t                edge        :1;     /* ET mod */
    
    qp_uint32_t                closed      :1;     /* need close */
    qp_uint32_t                listen      :1;     /* listen flag */
    qp_uint32_t                shutdown    :1;     /* source is closed */
    qp_uint32_t                urgen       :1;
    qp_uint32_t                write       :1;     /* write event */
    qp_uint32_t                read        :1;     /* read event */
    qp_uint32_t                write_again :1;     /* need write again */
    qp_uint32_t                read_again  :1;     /* need read again */
    qp_uint32_t                stat        :6;
    qp_uint32_t                            :16;
};

typedef struct qp_event_source_s*      qp_event_source_t;

struct  qp_event_s {
    struct qp_list_s           ready;         /* event ready list */
    struct qp_list_s           listen_ready;  /* event listen list */
    struct qp_rbtree_s         timer;
    struct qp_fd_s             event_fd;          
    struct qp_pool_s           event_pool;    /* event mem pool */
    struct qp_pool_s           source_cache_pool; /* cache pool */
    qp_epoll_event_t           bucket;        /* ready bucket */
    qp_int_t                   eventpool_size;    /* event pool size */
    qp_int_t                   source_cachepool_size; /* cache pool size */
    qp_int_t                   bucket_size;   /* ready bucket size */
    qp_event_idle_handler      idle;          /* idle callback */
    void*                      idle_arg;      /* idle event callback arg */
    qp_uchar_t                 read_cache[QP_EVENT_READCACHE_SIZE];
    bool                       is_alloced; 
    bool                       is_run;
};

#define QP_EVENT_SET_ALLOCED(event)   (((qp_event_t)(event))->is_alloced=true)
#define QP_EVENT_UNSET_ALLOCED(event) (((qp_event_t)(event))->is_alloced=false)


# ifndef  QP_OS_LINUX
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

# endif

bool
qp_event_is_alloced(qp_event_t event) 
{ return event ? event->is_alloced : false; }

/**
 * Create an epoll event manager.
 * 
 * @param event
 * @param size
 * @return 
 */
qp_int_t
qp_event_epoll_create(qp_event_t event, qp_int_t size)
{
    if (!event) {
        return QP_ERROR;
    }
    
#ifdef QP_OS_LINUX
    event->event_fd.fd = epoll_create(size);
#else 
    event->event_fd.fd = QP_ERROR;
#endif
    return event->event_fd.fd;
}

/**
 * Waiting for comming events.
 * 
 * @param event
 * @param event_ready
 * @param maxevents
 * @param timeout
 * @return 
 */
qp_int_t
qp_event_epoll_wait(qp_event_t event, qp_epoll_event_t event_ready, 
    qp_int_t maxevents, qp_int_t timeout)
{
    if (!event || !event_ready) {
        return QP_ERROR;
    }
    
#ifdef QP_OS_LINUX
    return epoll_wait(event->event_fd.fd, event_ready, maxevents, timeout);
#else 
    return QP_ERROR;
#endif
}

/**
 * Add an event source to event system.
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t
qp_event_epoll_add(qp_event_t event, qp_event_source_t source)
{
    if (!event || !source) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
    
    if (source->noblock) {
        fcntl(source->source_fd, F_SETFL, fcntl(source->source_fd, F_GETFL) \
            | O_NONBLOCK);
    }
    
#ifdef  QP_OS_LINUX
    setter.data.ptr = source;
    setter.events = source->events;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_ADD, source->source_fd, \
        &setter);
#else
    return QP_ERROR;
#endif
}

/**
 * Reset an event source with flag.
 * 
 * @param event
 * @param source
 * @param flag
 * @return 
 */
qp_int_t
qp_event_epoll_reset(qp_event_t event, qp_event_source_t source, qp_int_t flag)
{
    if (!event || !source) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = source;
    setter.events = source->events | flag;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_MOD, source->source_fd, \
        &setter);
#else
    return QP_ERROR;
#endif
}

/**
 * Delete and event source from event system.
 *  
 * @param emodule
 * @param eventfd
 * @return 
 */
qp_int_t
qp_event_epoll_del(qp_event_t event, qp_event_source_t source)
{
    if (!event || !source) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
#ifdef  QP_OS_LINUX
    setter.data.ptr = source;
    setter.events = 0;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_DEL, source->source_fd, \
        &setter);
#else
    return QP_ERROR;
#endif
}

/**
 * Accept from a listen source.
 * 
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_accept(qp_event_source_t source)
{
    return source ? accept(source->source_fd, NULL, NULL) : QP_ERROR;
}

/**
 * Close an event source.
 * 
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_close(qp_event_source_t source)
{
    if (!source) {
        return QP_ERROR;
    }
    
    if (source->closed && (QP_FD_INVALID != source->source_fd)) {
        close(source->source_fd);
    }

    source->source_fd = QP_FD_INVALID;
    return QP_SUCCESS;
}

/**
 * Check an source should be closed.
 * 
 * @param source
 * @return Return QP_SUCCESS if do not need to be closed, otherwise close the 
 *         source and return QP_ERROR.
 */
qp_int_t
qp_event_source_check_close(qp_event_source_t source)
{
    if (!source) {
        return QP_ERROR;
    }
    
    if (source->shutdown) {
        qp_event_source_close(source);
        return QP_ERROR;
    }

    return QP_SUCCESS;
}

/**
 * Read data from a source.
 * 
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_read(qp_event_source_t source)
{
    if (!source || (QP_FD_INVALID == source->source_fd)) {
        return QP_ERROR;
    }
    
    size_t rest = 0;
    ssize_t ret = 0;
    source->cache_offset = 0;

    if (source->read) {

        do {
            rest = source->cache_size - source->cache_offset;
            ret = read(source->source_fd, source->cache + source->cache_offset, \
                rest);

            if (1 > ret) {
                source->read = 0;

                /* closed or some error happen */
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno \
                    || EINTR == errno))
                {
                    source->shutdown = 1;
                    return QP_ERROR;
                }

            } else {
                source->cache_offset += ret;
                
                if (source->cache_offset >= source->cache_size) {
                    source->read = 0;
                    source->read_again = 1;
                }
            }
        } while (source->read && source->edge);
    }

    /* clear flag if edge mod not enabled */
    source->read = 0;
    return QP_SUCCESS;
}

//qp_int_t
//qp_event_source_readv(qp_event_source_t source)
//{
//    if (!source || (QP_FD_INVALID == source->source_fd)) {
//        return QP_ERROR;
//    }
//    
//    ssize_t ret;
//
//    if (eventfd->read && eventfd->field.readbuf_max 
//        && (QP_FD_INVALID != eventfd->efd)) 
//    {
//        ret = readv(eventfd->efd, eventfd->field.readbuf.vector, 
//            eventfd->field.readbuf_max);
//        eventfd->read = 0;
//        eventfd->read_finish = 1;
//
//        if (1 > ret) {
//
//            if ((0 == ret) 
//                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
//            {
//                qp_event_close(eventfd);
//                return QP_ERROR;
//            }
//        }
//    }
//    
//
//    return QP_SUCCESS;
//}

/**
 * Write data to a source.
 * 
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_write(qp_event_source_t source)
{
    if (!source || (QP_FD_INVALID == source->source_fd)) {
        return QP_ERROR;
    }
    
    size_t rest = 0;
    ssize_t ret = 0;
    
    if (source->write && (0 < source->cache_offset)) {
        
        do {
            ret = write(source->source_fd, source->cache + rest, \
                source->cache_offset - rest);
            
            if (1 > ret) {
                source->write = 0;
                
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno \
                    || EINTR == errno))
                {
                    source->shutdown = 1;
                    return QP_ERROR;
                }
                
                source->write_again = 1;
                
            } else {
                rest += ret;
            }
        } while(source->write && source->edge);
    }
    
    source->write = 0;
    return QP_SUCCESS;
}

//qp_int_t
//qp_event_source_writev(qp_event_fd_t eventfd)
//{
//    if (!eventfd) {
//        return QP_ERROR;
//    }
//    
//    ssize_t ret;
//
//    if (eventfd->write && eventfd->field.writebuf_max 
//        && (QP_FD_INVALID != eventfd->efd)) 
//    {
//        ret = writev(eventfd->efd, eventfd->field.writebuf.vector, \
//            eventfd->field.writebuf_max);
//        eventfd->write = 0;
//        eventfd->write_finish = 1;
//
//        if (1 > ret) {
//
//            if ((0 == ret) 
//                || !(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno))
//            {
//                qp_event_close(eventfd);
//                return QP_ERROR;
//            }
//        }
//    }
//    
//    return QP_SUCCESS;
//}

/**
 * Clear flag of the source.
 * 
 * @param source
 */
void
qp_event_source_clear_flag(qp_event_source_t source)
{
    if (!source) {
        return;
    }
    
    source->listen = 0;
    source->closed = 0;
    source->shutdown = 0;
    source->write = 0;
    source->write_again = 0;
    source->read = 0;
    source->read_again = 0;
    source->stat = QP_EVENT_IDL;
}

qp_event_t
qp_event_create(qp_event_t event)
{
    if (NULL == event) {
        event = (qp_event_t)qp_alloc(sizeof(struct qp_event_s));
        
        if (NULL == event) {
            return NULL;
        }
        
        memset(event, 0, sizeof(struct qp_event_s));
        QP_EVENT_SET_ALLOCED(event);
        
    } else {
        memset(event, 0, sizeof(struct qp_event_s));
    }
    
    if (NULL == qp_fd_init(&event->event_fd, QP_FD_TYPE_EVENT, false)) {
        qp_event_is_alloced(event) ? qp_free(event) : 1;
        return NULL;
    }
    
    qp_list_init(&event->ready);
    qp_list_init(&event->listen_ready);
    qp_rbtree_init(&event->timer);
    return event;
}

qp_event_t
qp_event_init(qp_event_t event, qp_int_t max_event_size, bool noblock, bool edge)
{
#ifndef QP_OS_LINUX
    return NULL;
#endif
    
    qp_int_t   mod = QP_EPOLL_IN | QP_EPOLL_RDHUP /*| QP_EPOLL_OUT*/ \
        | (edge ? QP_EPOLL_ET : 0);
    
    if (1 > max_event_size) {
        return NULL;
    }
    
    if (NULL == (event = qp_event_create(event))) {
        return NULL;
    }
    
    event->eventpool_size = max_event_size;
    event->source_cachepool_size = max_event_size;
    event->bucket_size = max_event_size;

    /* create epoll fd */
   if (QP_FD_INVALID == qp_event_epoll_create(event, max_event_size)) {
        qp_event_destroy(event);
        return NULL;
    }
    event->bucket = (qp_epoll_event_t)\
        qp_alloc(sizeof(struct qp_epoll_event_s) * event->bucket_size);
    
    /* init event pool */
    if (!event->bucket 
        || !qp_pool_init(&event->event_pool, sizeof(struct qp_event_source_s),\
        event->eventpool_size)
        || !qp_pool_init(&event->source_cache_pool, QP_EVENT_READCACHE_SIZE, \
        event->source_cachepool_size))
    {
        qp_event_destroy(event);
        return NULL;
    }
    
    for (int i = 0; i < event->eventpool_size; i++) {
        qp_event_source_t source = (qp_event_source_t)\
            qp_pool_to_array(&event->event_pool, i);
        
        source->index = i;
        source->source_fd = QP_FD_INVALID;
        source->events = mod;
        source->noblock = noblock;
        source->edge = mod & QP_EPOLL_ET;
        qp_event_source_clear_flag(source);
        
        /* set cache */
        source->cache = (qp_uchar_t*)\
            qp_pool_to_array(&event->source_cache_pool, i);
        qp_list_init(&source->ready_next);
        memset(&source->timer_node, 0, sizeof(struct qp_rbtree_node_s));
    }
  
    return event;
}

/**
 * Regist a handler that will be called when no events comming.
 * 
 * @param event
 * @param idle_cb
 * @param idle_arg
 * @return 
 */
qp_int_t
qp_event_regist_idle_handler(qp_event_t event, qp_event_idle_handler idle_cb, \
    void* idle_arg) 
{
    if (event && qp_fd_is_inited(&event->event_fd)) {
        event->idle = idle_cb;
        event->idle_arg = idle_arg;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
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


void
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
