/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "qp_event_module/qp_event_module.h"


typedef  size_t (*qp_read_handler)(qp_event_t, qp_event_source_t);
typedef  qp_read_handler       qp_write_handler;


static inline void 
qp_event_set_alloced(qp_event_t event) 
{
    event->is_alloced = true;
}


static inline void 
qp_event_unset_alloced(qp_event_t event) 
{
    event->is_alloced = false;
}


static inline void
qp_event_source_set_shutdown(qp_event_source_t source) 
{
    source->shutdown = 1;
    source->stat = QP_EVENT_CLOSE;
}


static inline bool
qp_event_is_alloced(qp_event_t event) 
{ 
    return event ? event->is_alloced : false; 
}

static inline qp_int_t
qp_event_source_accept(qp_event_source_t source)
{
    return accept(source->source_fd, NULL, NULL);
}


qp_int_t
qp_event_source_close(qp_event_source_t source)
{
    if (source->closed && (QP_FD_INVALID != source->source_fd)) {
        close(source->source_fd);
    }

    source->source_fd = QP_FD_INVALID;
    return QP_SUCCESS;
}


qp_int_t
qp_event_source_alloc_read_cache(qp_event_t event, qp_event_source_t source) 
{
    if (source->read_cache) {
        return QP_ERROR;
    }
    
    source->read_cache_size = QP_EVENT_READCACHE_SIZE;
    source->read_cache = \
          (qp_uchar_t*)qp_pool_alloc(&event->source_cache_pool, \
          source->read_cache_size);
    
    if (!source->read_cache) {
        source->read_cache_size = 0;
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}


qp_int_t
qp_event_source_free_read_cache(qp_event_t event, qp_event_source_t source) 
{
    if (!source->read_cache) {
        return QP_ERROR;
    }
    
    qp_pool_free(&event->source_cache_pool, source->read_cache);
    source->read_cache_size = 0;
    source->read_cache = NULL;
    return QP_SUCCESS;
}


qp_int_t 
qp_event_source_alloc_write_cache(qp_event_t event, qp_event_source_t source) 
{
    if (source->write_cache) {
        return QP_ERROR;
    }
    
    source->write_cache_size = QP_EVENT_READCACHE_SIZE;
    source->write_cache_cur_offset = 0;
    source->write_cache = (qp_uchar_t*)qp_pool_alloc(&event->source_cache_pool,\
        source->write_cache_size);
    
    if (!source->write_cache) {
        source->write_cache_size = 0;
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}


qp_int_t
qp_event_source_free_write_cache(qp_event_t event, qp_event_source_t source)
{
    if (!source->write_cache) {
        return QP_ERROR;
    }
    
    qp_pool_free(&event->source_cache_pool, source->write_cache);
    source->write_cache_size = 0;
    source->write_cache = NULL;
    return QP_SUCCESS;
}


size_t
qp_event_source_read(qp_event_t event, qp_event_source_t source)
{
    source->read_cache_offset = 0;
    
    if (source->read) {
        size_t  rest = 0;
        ssize_t ret = 0;
        source->read = 0;
        
        if (QP_ERROR == qp_event_source_alloc_read_cache(event, source)) {
            qp_event_source_set_shutdown(source);
            return QP_ERROR;
        }
         
        rest = source->read_cache_size - source->read_cache_offset;
        
        do {
            ret = read(source->source_fd, \
                source->read_cache + source->read_cache_offset, rest);
                
            if (1 > ret) {
                /* closed or some error happen */
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno \
                    || EINTR == errno))
                {
                    qp_event_source_set_shutdown(source);
                    return QP_ERROR;
                }
                break;

            } else {
                source->read_cache_offset += ret;
                rest = source->read_cache_size - source->read_cache_offset;
                 
                // read cache full
                if (0 >= rest) {
                    if (event->event_handle.qp_event_ev_reset) {
                        event->event_handle.qp_event_ev_reset(event, source, \
                            source->write_cache ? QP_EPOLL_OUT : 0);
                    }
                    break;
                }
            }
        } while (source->edge);
    } 
    
    /* clear flag if edge mod not enabled */
    return (size_t)source->read_cache_offset;
}


size_t
qp_event_source_write(qp_event_t event, qp_event_source_t source)
{
    if (!source->write_cache) {
        return QP_ERROR;
    }
    
    size_t  rest = source->write_cache_cur_offset;
        
    if (source->write) {
        ssize_t ret = source->write_cache_offset - rest;
        source->write = 0;
        
        do {
            ret = write(source->source_fd, source->write_cache + rest, ret);
            
            if (1 > ret) {
                
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno \
                    || EINTR == errno) || source->write_close)
                {
                    qp_event_source_set_shutdown(source);
                    qp_event_source_free_read_cache(event, source);
                    source->write_close = 0;
                    source->write = 0;
                    return QP_ERROR;
                }
                
                source->write_cache_cur_offset = rest;
                break;
                
            } else {
                rest += ret;
                ret = source->write_cache_offset - rest;
                
                // write finish
                if (0 >= ret) {
                    qp_event_source_free_write_cache(event, source);
                    break;
                }
            }
        } while(source->edge);
    }
    
    return (size_t)rest;
}


void
qp_event_source_clear_flag(qp_event_source_t source)
{
    source->listen = 0;
    source->closed = 0;
    source->shutdown = 0;
    source->urgen = 0;
    source->write = 0;
    source->read = 0;
    source->write_close = 0;
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
        qp_event_set_alloced(event);
        
    } else {
        memset(event, 0, sizeof(struct qp_event_s));
        qp_event_unset_alloced(event);
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
    qp_uint32_t   mod = QP_EPOLL_IN | QP_EPOLL_RDHUP /*| QP_EPOLL_OUT*/ \
        | QP_EPOLL_EXCLUSIVE | (edge ? QP_EPOLL_ET : 0);
    size_t  itr = 0;
    
    if (1 > max_event_size) {
        return NULL;
    }
    
    if (NULL == (event = qp_event_create(event)) 
        || !event->event_handle.qp_event_ev_create) 
    {
        return NULL;
    }
    
    /* create epoll fd */
    if (QP_FD_INVALID == \
        event->event_handle.qp_event_ev_create(event, max_event_size)) 
    {
        qp_event_destroy(event);
        return NULL;
    }
    
    event->eventpool_size = max_event_size;
    event->bucket_size = max_event_size;
    event->source_cachepool_size = max_event_size / 2;
#if !defined(QP_OS_LINUX)
# if defined(QP_OS_BSD)
    event->changelist_size = max_event_size;
    event->changelist = (qp_evpoll_event_t)\
        qp_alloc(sizeof(qp_evpoll_event_s) * event->changelist_size);
    if (!event->changelist) {
        qp_event_destroy(event);
        return NULL;
    }
# else 
# endif
#endif
    event->bucket = (qp_evpoll_event_t)\
        qp_alloc(sizeof(qp_evpoll_event_s) * event->bucket_size);
    
    /* init event pool */
    if (!event->bucket 
        || !qp_pool_init(&event->event_pool, sizeof(struct qp_event_source_s),\
        event->eventpool_size * 2)
        || !qp_pool_init(&event->source_cache_pool, \
           QP_EVENT_READCACHE_SIZE, event->source_cachepool_size))
    {
        qp_event_destroy(event);
        return NULL;
    }
    
    for (itr = 0; itr < event->eventpool_size * 2; itr++) {
        qp_event_source_t source = (qp_event_source_t)\
            qp_pool_to_array(&event->event_pool, itr);
        memset(source, 0, sizeof(struct qp_event_source_s));
        source->index = itr;
        source->source_fd = QP_FD_INVALID;
        source->events = mod;
        source->noblock = noblock;
        source->edge = mod & QP_EPOLL_ET;
        qp_list_init(&source->ready_next);
    }
  
    return event;
}


qp_int_t
qp_event_removeevent(qp_event_t event, qp_event_source_t source)
{
    if (!event || !source) {
        return QP_ERROR;
    }
    
    if (event->event_handle.qp_event_ev_del) {
        event->event_handle.qp_event_ev_del(event, source);
    }
    qp_event_source_close(source);
    qp_event_source_free_read_cache(event, source);
    qp_event_source_free_write_cache(event, source);
    qp_pool_free(&event->event_pool, source); 
    return QP_SUCCESS;
}


qp_int_t
qp_event_addevent(qp_event_t event, qp_int_t fd, qp_int_t timeout, bool listen, \
    bool auto_close)
{ 
    qp_event_source_t source = NULL;
    
    if (!event || !event->event_handle.qp_event_ev_add) {
        return QP_ERROR;
    }
    
    if (qp_pool_available(&event->event_pool) && (fd > QP_FD_INVALID)) {
        /* get the first idle element */
        source = (qp_event_source_t)qp_pool_alloc(&event->event_pool, \
            sizeof(struct qp_event_source_s));
        qp_event_source_clear_flag(source);
        source->listen = listen;
        source->closed = auto_close;
        source->source_fd = fd;
        source->timeout = timeout;

        /* add event to pool */
        if (QP_ERROR == event->event_handle.qp_event_ev_add(event, source)) {
            source->source_fd = QP_FD_INVALID;
            qp_event_removeevent(event, source);
            return QP_ERROR;
        }
        
        if (source->listen) {
            source->stat = QP_EVENT_PROCESS;
            
        } else {
            source->stat = QP_EVENT_NEW;
        }
        
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}


qp_int_t
qp_event_destroy(qp_event_t event)
{
    if (event && qp_fd_is_inited(&event->event_fd) && !event->is_run) { 
        qp_event_source_t source = NULL;
        size_t  itr = 0;
        qp_fd_destroy(&event->event_fd);
        
        for (itr = 0; itr < event->eventpool_size; itr++) {
            source = (qp_event_source_t)qp_pool_to_array(&event->event_pool, itr);
            qp_event_removeevent(event, source);
        }
        
        if (event->bucket) {
            qp_free(event->bucket);
            event->bucket = NULL;
            event->bucket_size = 0;
        }
        
#if !defined(QP_OS_LINUX)
# if defined(QP_OS_BSD)
        if (event->changelist) {
            qp_free(event->changelist);
            event->changelist = NULL;
            event->changelist_size = 0;
        }
# else 
# endif
#endif
        
        qp_pool_destroy(&event->event_pool, true);
        qp_pool_destroy(&event->source_cache_pool, true);
        
        if (qp_event_is_alloced(event)) {
            qp_free(event);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


qp_int_t
qp_event_regist_process_handler(qp_event_t event, qp_event_process_handle handle)
{
    if (event && qp_fd_is_inited(&event->event_fd)) {
        event->process_handle = handle;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


void
qp_event_dispatch_listen_queue(qp_event_t event, qp_int_t timeout) {
    qp_event_source_t  source = NULL;
    qp_int_t           sys_accept_fd = QP_FD_INVALID;
    
    while (!qp_list_is_empty(&event->listen_ready)) {
        source = (qp_event_source_t)qp_list_data(qp_list_first(&event->listen_ready), \
            struct qp_event_source_s, ready_next);
        qp_list_pop(&event->listen_ready);
            
        do {
            sys_accept_fd = qp_event_source_accept(source);
                
            if (QP_FD_INVALID == sys_accept_fd) {
                if (!(EAGAIN == errno|| EWOULDBLOCK == errno|| EINTR == errno)){
                    qp_event_removeevent(event, source);
                }
                    
                break;
            }
                
            /* should be auto closed */
            if (QP_ERROR ==  qp_event_addevent(event, sys_accept_fd, \
                timeout, false, true)) 
            {
                close(sys_accept_fd);
            }
                
        } while(source->edge);
    }
}


void
qp_event_dispatch_queue(qp_event_t event) {
    qp_event_source_t  source = NULL;
    qp_read_handler    read_handler = 0;
    qp_write_handler   write_handler = 0;
    qp_int_t           ret = 0;
    
    while (!qp_list_is_empty(&event->ready)) {
        source = (qp_event_source_t)qp_list_data(qp_list_first(&event->ready), \
            struct qp_event_source_s, ready_next);
        qp_list_pop(&event->ready);
           
        source->shutdown |= source->revents \
            & (QP_EPOLL_RDHUP | QP_EPOLL_HUP | QP_EPOLL_ERR);
        source->read |= source->revents | QP_EPOLL_IN;
        source->write |= source->revents | QP_EPOLL_OUT;
            
        if (!(source->shutdown | source->read | source->write)) {
            continue;
        }
            
        if (source->shutdown) {
            qp_event_source_set_shutdown(source);
        }
            
        read_handler = qp_event_source_read;
        write_handler = qp_event_source_write;
            
        write_handler(event, source);
        read_handler(event, source);
        
        // if write event process done
        if (!source->write_cache) {
            // read event
            if (source->read_cache) {
                if (event->process_handle.read) {
                    ret = event->process_handle.read(source->index, source->stat,\
                        source->read_cache, source->read_cache_offset);
                    
                } else {
                    ret = 0;
                }
            
                qp_event_source_free_read_cache(event, source);
                
                // change state
                if (QP_EVENT_NEW == source->stat && (ret > QP_ERROR)) {
                    source->stat = QP_EVENT_PROCESS;
                }
                
                // need close
                if (ret < 0) {
                    qp_event_source_set_shutdown(source);
                }
            
                // have data to be sent, alloc cache for write event
                if ((ret > 0) 
                    && !source->shutdown 
                    && event->process_handle.write) 
                {
                    if (QP_ERROR == \
                        qp_event_source_alloc_write_cache(event, source)) 
                    {
                        qp_event_source_set_shutdown(source);
                    }
                }
            }
            
            // fill write cache 
            if (source->write_cache) {
                ret = event->process_handle.write(source->index, source->stat,\
                    source->write_cache, &source->write_cache_offset,\
                    source->write_cache_size);
                
                if (0 > ret) {
                    qp_event_source_set_shutdown(source);
                    qp_event_source_free_write_cache(event, source);
                    
                } else {
                    if (event->event_handle.qp_event_ev_reset) {
                        event->event_handle.qp_event_ev_reset(event, source, \
                            QP_EPOLL_OUT);
                    }
                    
                    if (0 == ret) {
                        source->write_close = 1;
                    }
                }
                
            }
        } 
        
        if (source->shutdown) {
            qp_event_removeevent(event, source);
        }
    }
}


qp_int_t
qp_event_dispatch(qp_event_t event, qp_int_t timeout) 
{
    qp_event_source_t  source = NULL;
    qp_int_t           revent_num = 0;
    qp_int_t           itr = 0;
    if (!event || !qp_fd_is_valid(&event->event_fd) 
        || !event->event_handle.qp_event_ev_wait) 
    {
        return QP_ERROR;
    }
    
    event->is_run = true;
    
    while (event->is_run) {
        revent_num = event->event_handle.qp_event_ev_wait(event, timeout);
        
        /* idle or error */
        if (1 > revent_num) {
            if ((QP_ERROR == revent_num) && (EINTR != errno)) {
                /* error quit */
                break;
            }
            
            /* no error beacuse no event happen */
            if (event->process_handle.idle) {
                event->process_handle.idle(event->process_handle.idle_arg);
            }

            continue;
        }
        
        /* dispatch events */
        for (itr = 0; itr < revent_num; itr++) {
            //source = (qp_event_source_t)event->bucket[itr].data.ptr;
            //source->revents = event->bucket[itr].events;
            qp_list_push(source->listen ? &event->listen_ready : &event->ready,\
                &source->ready_next);
        }
        
        /* listen event */
        qp_event_dispatch_listen_queue(event, timeout);
        
        /* read/write event */
        qp_event_dispatch_queue(event);
    }
    
    event->is_run = false;
    return QP_SUCCESS;
}
