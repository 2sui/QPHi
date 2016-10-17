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


#include "qp_socket.h"
#include "core/qp_io_core.h"
#include "core/qp_memory_core.h"


struct  qp_socket_s {
    struct qp_fd_s                socket;      /* fd */
    union {
        struct sockaddr_in        inet_addr;
        struct sockaddr_in6       inet6_addr;
        struct sockaddr_un        unet_addr;
    }                             socket_addr;

    socklen_t                     socket_len;  /* socket addr length */
    qp_socket_domain_t            domain;      /* AF_INET or AF_INET6 or AF_UNIX */
    qp_socket_type_t              type;        /* stream or dgram or raw */
    qp_int_t                      backlog;     /* listening backlog */
    bool                          is_listen;   /* server or client */
    bool                          is_alloced;  /* is alloced */
};


static inline void
qp_socket_set_alloced(qp_socket_t skt)
{ 
    skt->is_alloced = true;
}


static inline void
qp_socket_set_listen(qp_socket_t skt)
{ 
    skt->is_listen = true;
}


static inline void
qp_socket_unset_listen(qp_socket_t skt)
{ 
    skt->is_listen = false;
}


static inline bool
qp_socket_is_alloced(qp_socket_t skt) 
{ 
    return skt->is_alloced; 
}


static inline bool
qp_socket_is_listen(qp_socket_t skt) 
{ 
    return skt->is_listen; 
}


qp_socket_t
qp_socket_assign_inet(qp_socket_t skt, const qp_char_t* name, qp_ushort_t port);


qp_socket_t
qp_socket_assign_inet6(qp_socket_t skt, const qp_char_t* name, qp_ushort_t port);


qp_socket_t
qp_socket_assign_unix(qp_socket_t skt, const qp_char_t* name);


qp_socket_t 
qp_socket_assign_packet(qp_socket_t skt);


qp_socket_t
qp_socket_create(qp_socket_t skt) {
    
    /* alloc struct if skt is NULL */
    if (NULL == skt) {
        skt = (qp_socket_t)qp_alloc(sizeof(struct qp_socket_s));
        
        if (NULL == skt) {
            return NULL;
        }

        memset(skt, 0, sizeof(struct qp_socket_s));
        qp_socket_set_alloced(skt);

    } else {
        memset(skt, 0, sizeof(struct qp_socket_s));
    }

    /* init fd */
    if (NULL == qp_fd_init(&skt->socket, QP_FD_TYPE_SOCKET, false)) {

        if (qp_socket_is_alloced(skt)) {
            qp_free(skt);
        }

        return NULL;
    }

    skt->domain = QP_SOCKET_DOMAIN_UNSUPPORT;
    skt->type = QP_SOCKET_TYPE_UNSUPPORT;
    return skt;
}


qp_socket_t
qp_socket_init(qp_socket_t skt, qp_int_t domain, qp_int_t type, 
    const qp_char_t* name, qp_ushort_t port, \
    bool as_server, qp_int_t server_backlog)
{
    /* check args */
    switch(domain) {

    case QP_SOCKET_DOMAIN_INET:
    case QP_SOCKET_DOMAIN_INET6: {

        if ((NULL == name) || (0 == port)) {
            return NULL;
        }
        
    }break;

    case QP_SOCKET_DOMAIN_UNIX: {

        if ((NULL == name) || (strlen(name) >= \
            sizeof(skt->socket_addr.unet_addr.sun_path))) 
        {
            return NULL;
        }
        
    }break;
#if !defined(QP_OS_BSD)
    case QP_SOCKET_DOMAIN_PACKET: {
        return NULL; // NOT SUPPORTED
    }break;
#endif

    default: {
        return NULL;
    }
    }
    
    switch (type) {

    case QP_SOCKET_TYPE_STREAM:
    case QP_SOCKET_TYPE_DGRAM:
    case QP_SOCKET_TYPE_RAW:
        break;
        
    default: {
        return NULL;
    }
        break;
    }

    /* create socket */
    skt = qp_socket_create(skt);
    
    if (NULL == skt) {
        return NULL;
    }

    skt->domain = domain;
    skt->type = type;

    if (as_server) {
        qp_socket_set_listen(skt);
        skt->backlog = server_backlog > 0 ? server_backlog : \
            QP_SOCKET_DEFAULT_LISTENBACKLOG;
    }

    /* get socket */
    skt->socket.fd = socket(skt->domain, skt->type, 0);
    
    if (!qp_fd_is_valid(&skt->socket)) {
        qp_socket_destroy(skt);
        return NULL;
    }

    qp_socket_t assign = NULL;
    
    switch (skt->domain) {

    case QP_SOCKET_DOMAIN_INET: {
        assign = qp_socket_assign_inet(skt, name, port);
        
    }break;

    case QP_SOCKET_DOMAIN_INET6: {
        assign = qp_socket_assign_inet6(skt, name, port);
        
    }break;

    case QP_SOCKET_DOMAIN_UNIX: {
        assign = qp_socket_assign_unix(skt, name);
        
    }break;
#if !defined(QP_OS_BSD)
    case QP_SOCKET_DOMAIN_PACKET:{
        assign = qp_socket_assign_packet(skt);
    }break;
#endif

    default: {
    }break;
    }
    
    if (assign) {
        return assign;
    }
    
    qp_socket_destroy(skt);
    return NULL;
}


qp_int_t
qp_socket_destroy(qp_socket_t skt)
{
    if (skt && qp_fd_is_inited(&skt->socket)) {
        qp_socket_close(skt, QP_SOCKET_SHUT_CLOSE);
        qp_fd_destroy(&skt->socket);
        skt->domain = QP_SOCKET_DOMAIN_UNSUPPORT;
        skt->type = QP_SOCKET_TYPE_UNSUPPORT;

        if (qp_socket_is_alloced(skt)) {
            qp_free(skt);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


qp_socket_t
qp_socket_assign_inet(qp_socket_t skt, const qp_char_t* name, qp_ushort_t port) 
{
    memset(&skt->socket_addr, 0, sizeof(struct sockaddr_in));
        skt->socket_addr.inet_addr.sin_family = skt->domain;

    /* listen */
    if (qp_socket_is_listen(skt)) {

        if (0 == strcmp(name, "0.0.0.0")) {
            skt->socket_addr.inet_addr.sin_addr.s_addr = INADDR_ANY;
            
        } else {
            int ret = inet_pton(skt->domain, name, 
                &skt->socket_addr.inet_addr.sin_addr);
                
            if (1 != ret) {
                return NULL;
            }
        }

        skt->socket_len = sizeof(struct sockaddr_in);
        skt->socket_addr.inet_addr.sin_port = ntohs(port);

        /* no listen */
    } else {
        int ret = inet_pton(skt->domain, \
            name,&skt->socket_addr.inet_addr.sin_addr);
            
        if (1 != ret) {
            return NULL;
        }

        skt->socket_len = sizeof(struct sockaddr_in);
        skt->socket_addr.inet_addr.sin_port = htons(port);
    }

    return skt;
}


qp_socket_t
qp_socket_assign_inet6(qp_socket_t skt, const qp_char_t* name, qp_ushort_t port) 
{
    return NULL;
    
    // NOT SUPPORTED
    skt = NULL;
    name = NULL;
    port = 0;
    return NULL;
}


qp_socket_t
qp_socket_assign_unix(qp_socket_t skt, const qp_char_t* name) 
{
    memset(&skt->socket_addr.unet_addr, 0, sizeof(struct sockaddr_un));
    skt->socket_addr.unet_addr.sun_family = skt->domain;
    
    if (qp_socket_is_listen(skt)) {
        strcpy(skt->socket_addr.unet_addr.sun_path, name);
        skt->socket_len = offsetof(struct sockaddr_un, sun_path) \
            +strlen(skt->socket_addr.unet_addr.sun_path);

        /* delete exit file */
        unlink(skt->socket_addr.unet_addr.sun_path);

    } else {
        socklen_t len;
        struct sockaddr_un saddr;

        memset(&saddr, 0, sizeof(struct sockaddr_un));
        saddr.sun_family = skt->domain;

        /* client descriptor */
        sprintf(saddr.sun_path, "%s%05ld", QP_SOCKET_DEFAULT_UNET_PATH,\
            (long)pthread_self());
        len = offsetof(struct sockaddr_un, sun_path) + strlen(saddr.sun_path);

        /* delete exit file */
        unlink(saddr.sun_path);

        /* bind scoket */
        if (QP_SUCCESS != bind(qp_fd_get_fd(&skt->socket), \
            (struct sockaddr*)&saddr, len)) 
        {
            return NULL;
        }

        /* owner only */
        if (QP_SUCCESS != chmod(saddr.sun_path, S_IRWXU)) {
            unlink(saddr.sun_path);
            return NULL;
        }

        /* set server descriptor */
        strcpy(skt->socket_addr.unet_addr.sun_path, name);
        skt->socket_len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
    }

    return skt;
}

#if !defined(QP_OS_BSE)
qp_socket_t 
qp_socket_assign_packet(qp_socket_t skt) 
{
    /* NOT SUPPORTED */
    return NULL;
    
    if (!skt) {
        return NULL;
    }
    
    return NULL;
}
#endif

qp_int_t
qp_socket_to_int(qp_socket_t skt)
{
    if (skt && qp_fd_is_valid(&skt->socket)) {
        return skt->socket.fd;
    }
    
    return QP_ERROR;
}


qp_int_t
qp_socket_close(qp_socket_t skt, qp_socket_shut_t shut)
{
    if (skt && qp_fd_is_valid(&skt->socket)) {
        
        /* close socket at once */
        if (shut != QP_SOCKET_SHUT_CLOSE) {
            shutdown(qp_fd_get_fd(&skt->socket), shut);
        }

        return qp_fd_close(&skt->socket);
    }
    
    return QP_ERROR;
}


qp_int_t
qp_socket_listen(qp_socket_t skt, qp_int_t mod)
{
    if (!qp_socket_is_listen(skt) || !qp_fd_is_valid(&skt->socket)) {
        return QP_ERROR;
    }
    
    if (QP_SUCCESS != bind(skt->socket.fd, \
        (struct sockaddr*)&skt->socket_addr, skt->socket_len)) {
        return QP_ERROR;
    }
    
    if (QP_SUCCESS != listen(skt->socket.fd, skt->backlog)) {
        return QP_ERROR;
    }
    
    /* set rd/wr privilege */
    if (skt->domain == QP_SOCKET_DOMAIN_UNIX) {
        chmod(skt->socket_addr.unet_addr.sun_path, mod);
    }

    return QP_SUCCESS;
}


qp_socket_t
qp_socket_accept(qp_socket_t skt, qp_socket_t sktClient)
{
    if (qp_socket_is_listen(skt) && qp_fd_is_valid(&skt->socket)) {
        
        if (NULL == (sktClient = qp_socket_create(sktClient))) {
            return NULL;
        }

        sktClient->domain = skt->domain;
        sktClient->type = skt->type;

        switch (sktClient->domain) {

        case QP_SOCKET_DOMAIN_UNIX:{
            sktClient->socket_len = sizeof(struct sockaddr_un);
        }break;

        case QP_SOCKET_DOMAIN_INET:{
            sktClient->socket_len = sizeof(struct sockaddr_in);
        }break;

        case QP_SOCKET_DOMAIN_INET6:{
            sktClient->socket_len = sizeof(struct sockaddr_in6);
        }break;
#if !defined(QP_OS_BSD)
        case QP_SOCKET_DOMAIN_PACKET:
#endif
        default:{
            sktClient->socket_len = sizeof(struct sockaddr);
        }break;
        }

        sktClient->socket.fd = accept(skt->socket.fd, \
            (struct sockaddr*)&sktClient->socket_addr, &sktClient->socket_len);
        return sktClient;
    }
    
    return NULL;
}


qp_int_t
qp_socket_connect(qp_socket_t skt)
{
    if (qp_socket_is_listen(skt) || !qp_fd_is_valid(&skt->socket)) {
        return QP_ERROR;
    }

    return connect(skt->socket.fd, (struct sockaddr*)&(skt->socket_addr),\
        skt->socket_len);
}


qp_int_t
qp_socket_setsockopt(qp_socket_t skt, qp_int_t level, qp_int_t optname, \
    const void* optval, socklen_t optlen)
{
    if (!skt || !qp_fd_is_valid(&skt->socket)) {
        return QP_ERROR;
    }
    
    return setsockopt(skt->socket.fd, level, optname, optval, optlen);
}


size_t
qp_socket_sendn(qp_socket_t skt, const void* vptr, size_t nbytes)
{
    return skt ? qp_fd_writen(&skt->socket, vptr, nbytes) : 0;
}


size_t
qp_socket_recvn(qp_socket_t skt, void* vptr, size_t nbytes)
{
    return skt ? qp_fd_readn(&skt->socket, vptr, nbytes) : 0;
}


ssize_t
qp_socket_sendv(qp_socket_t skt, const struct iovec* iov, qp_int_t iovcnt)
{
    return skt ? qp_fd_writev(&skt->socket, iov, iovcnt) : QP_ERROR;
}


ssize_t
qp_socket_recvv(qp_socket_t skt, const struct iovec* iov, qp_int_t iovcnt)
{
    return skt ? qp_fd_readv(&skt->socket, iov, iovcnt) : QP_ERROR;
}


ssize_t
qp_socket_send(qp_socket_t skt, const void* vptr, size_t nbytes, qp_int_t flag)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    return send(skt->socket.fd, vptr, nbytes, flag);
}


ssize_t
qp_socket_recv(qp_socket_t skt, void* vptr, size_t nbytes, qp_int_t flag)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    return recv(skt->socket.fd, vptr, nbytes, flag);
}


qp_int_t
qp_socket_set_noblock(qp_socket_t skt)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    return qp_fd_setNoBlock(&skt->socket);
}

/* option */
qp_int_t
qp_socket_set_reuse(qp_socket_t skt, qp_int_t reuse, qp_int_t enable)
{
    if (reuse == QP_SOCKET_SO_REUSE_ADDR) {
        return qp_socket_setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, \
            (const void *)&enable, sizeof(enable));
    }
#ifdef QP_OS_BSD
    if (reuse == QP_SOCKET_SO_REUSE_PORT) {
        return qp_socket_setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, \
            (const void *)&enable, sizeof(enable));
    }
#endif
    return QP_SUCCESS;
}


qp_int_t
qp_socket_set_nopush(qp_socket_t skt, qp_int_t enable)
{   
#ifdef QP_OS_BSD
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_NOPUSH, \
        (const void *)&enable, sizeof(enable));
#else
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_CORK, \
        (const void *)&enable, sizeof(enable));
#endif
}


qp_int_t
qp_socket_set_nodelay(qp_socket_t skt, qp_int_t enable)
{
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, \
        (const void *)&enable, sizeof(enable));
}

#if !defined(QP_OS_BSD)
qp_int_t
qp_socket_set_quickack(qp_socket_t skt, qp_int_t enable)
{
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_QUICKACK, \
        (const void *)&enable, sizeof(enable));
}
#endif
