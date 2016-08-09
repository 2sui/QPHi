
/**
  * Copyright (C) 2sui.
  */


#include "qp_event.h"
#include "qp_o_io.h"
#include "qp_o_pool.h"


# ifdef QP_OS_LINUX
typedef  struct epoll_event    qp_epoll_event_s;
# else 
typedef  void                  qp_epoll_event_s;
# endif

typedef  qp_epoll_event_s*     qp_epoll_event_t;

struct qp_event_source_s {
    struct qp_list_s           ready_next;     /* event source list */
    struct qp_rbtree_node_s    timer_node;            
    qp_int_t                   index;
    qp_int_t                   source_fd;
    qp_uint32_t                events;
    qp_uint32_t                revents;
    qp_uchar_t*                read_cache;          /* read/write cache */ 
    ssize_t                    read_cache_size;
    ssize_t                    read_cache_offset;
    qp_uchar_t*                write_cache;
    ssize_t                    write_cache_size;
    ssize_t                    write_cache_cur_offset;
    ssize_t                    write_cache_offset;
    qp_int_t                   timeout;
    
    qp_uint32_t                noblock     :1;     /* need noblock */
    qp_uint32_t                edge        :1;     /* ET mod */
    
    qp_uint32_t                closed      :1;     /* need close */
    qp_uint32_t                listen      :1;     /* listen flag */
    qp_uint32_t                shutdown    :1;     /* source is closed */
    qp_uint32_t                urgen       :1;
    qp_uint32_t                write       :1;     /* write event */
    qp_uint32_t                read        :1;     /* read event */
    qp_uint32_t                stat        :4;
    qp_uint32_t                            :20;
};

typedef struct qp_event_source_s*      qp_event_source_t;

struct  qp_event_s {
    struct qp_list_s           ready;         /* event ready list */
    struct qp_list_s           listen_ready;  /* event listen list */
    struct qp_rbtree_s         timer;
    struct qp_fd_s             event_fd;          
    struct qp_pool_s           event_pool;    /* event mem pool */
    struct qp_pool_manager_s   source_cache_pool; /* cache pool */
    qp_epoll_event_t           bucket;        /* ready bucket */
    qp_int_t                   eventpool_size;    /* event pool size */
    qp_int_t                   source_cachepool_size; /* cache pool size */
    qp_int_t                   bucket_size;   /* ready bucket size */
    qp_event_read_process_handler   read_process;
    qp_event_write_process_handler  write_process;
    qp_event_idle_handler      idle;          /* idle callback */
    void*                      idle_arg;      /* idle event callback arg */
    qp_uchar_t                 read_cache[QP_EVENT_READCACHE_SIZE];
    bool                       is_alloced; 
    bool                       is_run;
};

typedef  size_t (*qp_read_handler)(qp_event_t, qp_event_source_t);
typedef  qp_read_handler       qp_write_handler;


inline static void 
qp_event_set_alloced(qp_event_t event) 
{
    event->is_alloced = true;
}

inline static void 
qp_event_unset_alloced(qp_event_t event) 
{
    event->is_alloced = false;
}

inline static void
qp_event_source_set_shutdown(qp_event_source_t source) 
{
    source->shutdown = 1;
    source->stat = QP_EVENT_CLOSE;
}

inline static bool
qp_event_is_alloced(qp_event_t event) 
{ 
    return event ? event->is_alloced : false; 
}

# ifndef  QP_OS_LINUX
qp_int_t
epoll_create(int __size)
{
    return QP_ERROR;
}

qp_int_t
epoll_ctl(int __epfd, int __op, int __fd, epoll_event *__event)
{
    return QP_ERROR;
}

qp_int_t
epoll_wait(int __epfd, epoll_event *__events, int __maxevents, int __timeout)
{
    return QP_ERROR;
}

# endif

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
    
# ifdef QP_OS_LINUX
    event->event_fd.fd = epoll_create(size);
# else 
    event->event_fd.fd = QP_ERROR;
# endif
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
    
# ifdef QP_OS_LINUX
    return epoll_wait(event->event_fd.fd, event_ready, maxevents, timeout);
# else 
    return QP_ERROR;
# endif
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
    
# ifdef  QP_OS_LINUX
    setter.data.ptr = source;
    setter.events = source->events;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_ADD, source->source_fd, \
        &setter);
# else
    return QP_ERROR;
# endif
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
qp_event_epoll_reset(qp_event_t event, qp_event_source_t source, qp_uint32_t flag)
{
    if (!event || !source) {
        return QP_ERROR;
    }
    
    qp_epoll_event_s setter;
# ifdef  QP_OS_LINUX
    setter.data.ptr = source;
    setter.events = source->events | flag;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_MOD, source->source_fd, \
        &setter);
# else
    return QP_ERROR;
# endif
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
# ifdef  QP_OS_LINUX
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_DEL, source->source_fd, \
        &setter);
# else
    return QP_ERROR;
# endif
}

/**
 * Accept from a listen source.
 * 
 * @param source
 * @return 
 */
inline static qp_int_t
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
 * Set read cache for the source.
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_alloc_read_cache(qp_event_t event, qp_event_source_t source) 
{
    if (!event || !source || source->read_cache) {
        return QP_ERROR;
    }
    
    source->read_cache_size = QP_EVENT_READCACHE_SIZE;
    source->read_cache_offset = 0;
    source->read_cache = (qp_uchar_t*)\
    qp_pool_manager_alloc(&event->source_cache_pool, source->read_cache_size, \
        NULL);
    
    if (!source->read_cache) {
        source->read_cache_size = 0;
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}

/**
 * Free read cache for the source.
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_free_read_cache(qp_event_t event, qp_event_source_t source) 
{
    if (!event || !source || !source->read_cache) {
        return QP_ERROR;
    }
    
    qp_pool_manager_free(&event->source_cache_pool, source->read_cache, NULL);
    source->read_cache_size = 0;
    source->read_cache = NULL;
    return QP_SUCCESS;
}

/**
 * Set write cache for the source.
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t 
qp_event_source_alloc_write_cache(qp_event_t event, qp_event_source_t source) 
{
    if (!event || !source || source->write_cache) {
        return QP_ERROR;
    }
    
    source->write_cache_size = QP_EVENT_READCACHE_SIZE;
    source->write_cache_cur_offset = 0;
    source->write_cache_offset = 0;
    source->write_cache = (qp_uchar_t*)\
    qp_pool_manager_alloc(&event->source_cache_pool, source->write_cache_size, \
        NULL);
    
    if (!source->write_cache) {
        source->write_cache_size = 0;
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}

/**
 * Free write cache for the source.
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t
qp_event_source_free_write_cache(qp_event_t event, qp_event_source_t source)
{
    if (!event || !source || !source->write_cache) {
        return QP_ERROR;
    }
    
    qp_pool_manager_free(&event->source_cache_pool, source->write_cache, NULL);
    source->write_cache_size = 0;
    source->write_cache = NULL;
    return QP_SUCCESS;
}

/**
 * Read data from a source.
 * 
 * @param source
 * @return 
 */
size_t
qp_event_source_read(qp_event_t event, qp_event_source_t source)
{
    if (!source || (QP_FD_INVALID == source->source_fd) || !source->read_cache) {
        return QP_ERROR;
    }
    
    size_t rest = 0;
    ssize_t ret = 0;
    source->read_cache_offset = 0;
    rest = source->read_cache_size - source->read_cache_offset;

    if (source->read) {
        
        do {
            ret = read(source->source_fd, source->read_cache \
                + source->read_cache_offset, rest);
                
            if (1 > ret) {
                /* closed or some error happen */
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno \
                    || EINTR == errno))
                {
                    qp_event_source_set_shutdown(source);
                    source->read = 0;
                    return QP_ERROR;
                }
                break;

            } else {
                source->read_cache_offset += ret;
                rest = source->read_cache_size - source->read_cache_offset;
                    
                if (!rest) {
                    qp_event_epoll_reset(event, source, \
                        source->write_cache ? QP_EPOLL_OUT : 0);
                    break;
                }
            }
        } while (source->edge);
    }

    /* clear flag if edge mod not enabled */
    source->read = 0;
    return (size_t)source->read_cache_offset;
}

/**
 * Write data to a source.
 * 
 * @param source
 * @return 
 */
size_t
qp_event_source_write(qp_event_t event, qp_event_source_t source)
{
    if (!source || (QP_FD_INVALID == source->source_fd || !source->write_cache)) {
        return QP_ERROR;
    }
    
    size_t rest = source->write_cache_cur_offset;
    ssize_t ret = source->write_cache_offset - rest;
    
    if (source->write) {
        
        do {
            ret = write(source->source_fd, source->write_cache + rest, ret);
            
            if (1 > ret) {
                if ((0 == ret) || !(EAGAIN == errno || EWOULDBLOCK == errno \
                    || EINTR == errno))
                {
                    qp_event_source_set_shutdown(source);
                    source->write = 0;
                    return QP_ERROR;
                }
                
                source->write_cache_cur_offset = rest;
                break;
                
            } else {
                rest += ret;
                ret = source->write_cache_offset - rest;
                
                if (!ret) {
                    qp_event_source_free_write_cache(event, source);
                    break;
                }
            }
        } while(source->edge);
    }
    
    source->write = 0;
    return (size_t)rest;
}

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
    source->urgen = 0;
    source->write = 0;
    source->read = 0;
    source->stat = QP_EVENT_IDL;
}

/**
 * Create an event system.
 * 
 * @param event
 * @return 
 */
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

/**
 * Init an event system.
 * 
 * @param event
 * @param max_event_size
 * @param noblock
 * @param edge
 * @return 
 */
qp_event_t
qp_event_init(qp_event_t event, qp_int_t max_event_size, bool noblock, bool edge)
{
#ifndef QP_OS_LINUX
    return NULL;
#endif
    
    qp_uint32_t   mod = QP_EPOLL_IN | QP_EPOLL_RDHUP /*| QP_EPOLL_OUT*/ \
        | (edge ? QP_EPOLL_ET : 0);
    qp_int_t  itr = 0;
    
    if (1 > max_event_size) {
        return NULL;
    }
    
    if (NULL == (event = qp_event_create(event))) {
        return NULL;
    }
    
    event->eventpool_size = max_event_size;
    event->bucket_size = max_event_size;
    event->source_cachepool_size = max_event_size / 2;

    /* create epoll fd */
   if (QP_FD_INVALID == qp_event_epoll_create(event, max_event_size)) {
        qp_event_destroy(event);
        return NULL;
    }
    event->bucket = (qp_epoll_event_t)\
        qp_alloc(sizeof(qp_epoll_event_s) * event->bucket_size);
    
    /* init event pool */
    if (!event->bucket 
        || !qp_pool_init(&event->event_pool, sizeof(struct qp_event_source_s),\
        event->eventpool_size)
        || !qp_pool_manager_init(&event->source_cache_pool, \
        QP_EVENT_READCACHE_SIZE, event->source_cachepool_size))
    {
        qp_event_destroy(event);
        return NULL;
    }
    
    for (itr = 0; itr < event->eventpool_size; itr++) {
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

/**
 * Remove event source from event system .
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t
qp_event_removeevent(qp_event_t event, qp_event_source_t source)
{
    if (!event || !source) {
        return QP_ERROR;
    }
     
    qp_event_epoll_del(event, source);
    qp_event_source_close(source);
    qp_event_source_free_read_cache(event, source);
    qp_event_source_free_write_cache(event, source);
    qp_pool_free(&event->event_pool, source);
    return QP_SUCCESS;
}

/**
 * Add a fd to event system.
 * 
 * @param event
 * @param fd
 * @param timeout
 * @param listen
 * @param auto_close
 * @return 
 */
qp_int_t
qp_event_addevent(qp_event_t event, qp_int_t fd, qp_int_t timeout, bool listen, \
    bool auto_close)
{ 
    qp_event_source_t source = NULL;
    
    if (!event) {
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
        if (QP_ERROR == qp_event_epoll_add(event, source)) {
            source->source_fd = QP_FD_INVALID;
            qp_event_removeevent(event, source);
            return QP_ERROR;
        }
        
        if (source->listen) {
            source->stat = QP_EVENT_PROCESS;
            
        } else {
            source->stat = QP_EVENT_NEW;
            if (QP_ERROR == qp_event_source_alloc_read_cache(event, source)) {
                qp_event_removeevent(event, source);
                source->source_fd = QP_FD_INVALID;
                return QP_ERROR;
            }
        }
        
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}

/**
 * Destroy an event system.
 * 
 * @param emodule
 * @return 
 */
qp_int_t
qp_event_destroy(qp_event_t event)
{
    if (event && qp_fd_is_inited(&event->event_fd) && !event->is_run) { 
        qp_event_source_t source = NULL;
        qp_int_t  itr = 0;
        qp_fd_destroy(&event->event_fd);
        
        for (itr = 0; itr < event->eventpool_size; itr++) {
            source = (qp_event_source_t)qp_pool_to_array(&event->event_pool, itr);
            qp_event_removeevent(event, source);
        }
        
        if (event->bucket) {
            qp_free(event->bucket);
            event->bucket = NULL;
        }
        
        qp_pool_destroy(&event->event_pool, true);
        qp_pool_manager_destroy(&event->source_cache_pool, true);
        
        if (qp_event_is_alloced(event)) {
            qp_free(event);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
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

/**
 * Regist a hander that will be called when read events happen.
 * 
 * @param event
 * @param process
 * @return 
 */
qp_int_t
qp_event_regist_read_process_handler(qp_event_t event, \
    qp_event_read_process_handler process) 
{
    if (event && qp_fd_is_inited(&event->event_fd)) {
        event->read_process = process;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

/**
 * Regist a hander that will be called when write events happen.
 * 
 * @param event
 * @param process
 * @return 
 */
qp_int_t
qp_event_regist_write_process_handler(qp_event_t event, \
    qp_event_write_process_handler process)
{
    if (event && qp_fd_is_inited(&event->event_fd)) {
        event->write_process = process;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

/**
 * Dispatch listen events to listen queue.
 * 
 * @param event
 * @param timeout
 */
void
qp_event_dispatch_listen_queue(qp_event_t event, qp_int_t timeout) {
    qp_event_source_t  source = NULL;
    qp_int_t           sys_accept_fd = QP_FD_INVALID;
    
    while (!qp_list_is_empty(&event->listen_ready)) {
            source = qp_list_data(qp_list_first(&event->listen_ready), \
                struct qp_event_source_s, ready_next);
            qp_list_pop(&event->listen_ready);
            
            do {
                sys_accept_fd = qp_event_source_accept(source);
                
                if (QP_FD_INVALID == sys_accept_fd) {
                    if (!(EAGAIN == errno || EWOULDBLOCK == errno || \
                        EINTR == errno))
                    {
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

/**
 * Dispatch read/write events to event queue.
 * 
 * @param event
 */
void
qp_event_dispatch_queue(qp_event_t event) {
    qp_event_source_t  source = NULL;
    qp_read_handler    read_handler = 0;
    qp_write_handler   write_handler = 0;
    qp_int_t           ret = QP_ERROR;
    
    while (!qp_list_is_empty(&event->ready)) {
        source = qp_list_data(qp_list_first(&event->ready), \
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
        
        /* still have data to be sent */ 
        if (!source->write_cache) {
            if (event->read_process) {
                ret = event->read_process(source->index, source->stat, \
                    source->read_cache, source->read_cache_offset);
                
                if (0 > ret) {
                    qp_event_source_set_shutdown(source);
                }
                
                if (!source->shutdown && (ret > 0)) {
                    qp_event_source_alloc_write_cache(event, source);
                }
            }
            
            if (source->write_cache && event->write_process) {
                ret = event->write_process(source->index, source->stat, ret, \
                    source->write_cache, source->write_cache_offset);
                
                if (0 >= ret) {
                    qp_event_source_set_shutdown(source);
                    
                } else {
                    if (ret > source->write_cache_size) {
                        ret = source->write_cache_size;
                    }
                
                    source->write_cache_offset = ret;
                    source->write_cache_cur_offset = 0;
                    qp_event_epoll_reset(event, source, QP_EPOLL_OUT);
                }
            }
        }
        
        if (source->shutdown) {
            qp_event_removeevent(event, source);
        }
    }
}

/**
 * Events dispatch run loop.
 * 
 * @param event
 * @param timeout
 * @return 
 */
qp_int_t
qp_event_dispatch(qp_event_t event, qp_int_t timeout) 
{
    qp_event_source_t  source = NULL;
    qp_int_t           revent_num = 0;
    qp_int_t           itr = 0;
    if (!event || !qp_fd_is_valid(&event->event_fd)) {
        return QP_ERROR;
    }
    
    event->is_run = true;
    
    while (event->is_run) {
        revent_num = qp_event_epoll_wait(event, event->bucket, event->bucket_size,\
            timeout);
        /* idle or error */
        if (1 > revent_num) {
            if ((-1 == revent_num) && (EINTR != errno)) {
                /* error quit */
            }
            
            /* no error beacuse no event happen */
            if (event->idle) {
                event->idle(event->idle_arg);
            }

            continue;
        }
        
        /* dispatch events */
        for (itr = 0; itr < revent_num; itr++) {
            source = (qp_event_source_t)event->bucket[itr].data.ptr;
            source->revents = event->bucket[itr].events;
            qp_list_push(source->listen ? &event->listen_ready : &event->ready, \
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
