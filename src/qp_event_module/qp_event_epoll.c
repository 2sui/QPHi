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


#include "qp_event_module.h"


#if defined(QP_OS_LINUX)
/**
 * Create an epoll event manager.
 * 
 * @param event
 * @param size
 * @return 
 */
qp_int_t
qp_event_evpoll_create(qp_event_t event, qp_int_t size)
{
    return (event->event_fd.fd = epoll_create(size));
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
qp_event_evpoll_wait(qp_event_t event, qp_evpoll_event_t bucket, \
    qp_int_t bucket_size, qp_int_t timeout)
{
    return epoll_wait(event->event_fd.fd, bucket, bucket_size, timeout);
}


/**
 * Add an event source to event system.
 * 
 * @param event
 * @param source
 * @return 
 */
qp_int_t
qp_event_evpoll_add(qp_event_t event, qp_event_source_t source)
{
    if (source->noblock) {
        fcntl(source->source_fd, F_SETFL, \
            fcntl(source->source_fd, F_GETFL) | O_NONBLOCK);
    }
    
    qp_evpoll_event_s setter;
    setter.data.ptr = source;
    setter.events = source->events;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_ADD, \
        source->source_fd, &setter);
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
qp_event_evpoll_reset(qp_event_t event, qp_event_source_t source, qp_uint32_t flag)
{
    qp_evpoll_event_s setter;
    setter.data.ptr = source;
    setter.events = source->events | flag;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_MOD, \
        source->source_fd, &setter);
}


/**
 * Delete and event source from event system.
 *  
 * @param emodule
 * @param eventfd
 * @return 
 */
qp_int_t
qp_event_evpoll_del(qp_event_t event, qp_event_source_t source)
{
    qp_evpoll_event_s setter;
    return epoll_ctl(event->event_fd.fd, EPOLL_CTL_DEL, \
        source->source_fd, &setter); 
}
#endif
