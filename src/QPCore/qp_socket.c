
/**
  * Copyright (C) 2sui.
  */


#include "qp_socket.h"


inline void
qp_socket_set_alloced(qp_socket_t* skt)
{ skt ? skt->is_alloced = true : 1;}

inline void
qp_socket_set_listen(qp_socket_t* skt)
{ skt ? skt->is_listen = true : 1;}

inline void
qp_socket_unset_alloced(qp_socket_t* skt)
{ skt ? skt->is_alloced = false : 1;}

inline void
qp_socket_unset_listen(qp_socket_t* skt)
{ skt ? skt->is_listen = false : 1;}


inline bool
qp_socket_is_alloced(qp_socket_t* skt) 
{ return skt ? skt->is_alloced : false; }

inline bool
qp_socket_is_listen(qp_socket_t* skt) 
{ return skt ? skt->is_listen : false; }


qp_socket_t*
qp_socket_create(qp_socket_t* skt) {
    
    /* alloc struct if skt is NULL */
    if (NULL == skt) {
        skt = (qp_socket_t*)qp_alloc(sizeof(qp_socket_t));
        
        if (NULL == skt) {
            QP_LOGOUT_ERROR("[qp_socket_t] Socket create attr fail.");
            return NULL;
        }

        memset(skt, 0, sizeof(qp_socket_t));
        qp_socket_set_alloced(skt);

    } else {
        memset(skt, 0, sizeof(qp_socket_t));
    }

    /* init fd */
    if (NULL == qp_fd_init(&skt->socket, QP_FD_TYPE_SOCKET, false)) {

        if (qp_socket_is_alloced(skt)) {
            qp_free(skt);
        }

        QP_LOGOUT_ERROR("[qp_socket_t] Socket fd create fail.");
        return NULL;
    }

    skt->domain = QP_SOCKET_DOMAIN_UNSUPPORT;
    skt->type = QP_SOCKET_TYPE_UNSUPPORT;
    return skt;
}

qp_socket_t*
qp_socket_init(qp_socket_t* skt, qp_int_t domain, qp_int_t type, 
    const qp_char_t* name, qp_ushort_t port, bool as_server)
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

        if ((NULL == name) 
            || (strlen(name) >= sizeof(skt->socket_addr.unet_addr.sun_path))) 
        {
            return NULL;
        }
        
    }break;

    case QP_SOCKET_DOMAIN_PACKET: {
    }break;

    default: {
        QP_LOGOUT_ERROR("[qp_socket_t] Unknow domain.");
        return NULL;
    }
    }
    
    switch (type) {

    case QP_SOCKET_TYPE_STREAM:
    case QP_SOCKET_TYPE_DGRAM:
    case QP_SOCKET_TYPE_RAW:
        break;
        
    default: {
        QP_LOGOUT_ERROR("[qp_socket_t] Unknow socket type.");
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
        skt->backlog = QP_SOCKET_DEFAULT_LISTENBACKLOG;
    }

    /* get socket */
    skt->socket.fd = socket(skt->domain, skt->type, 0);
    skt->socket.errono = errno;

    if (!qp_fd_is_valid(&skt->socket)) {
        QP_LOGOUT_ERROR("[qp_socket_t] Socket setup fail.");
        qp_socket_destroy(skt);
        return NULL;
    }

    qp_socket_t* assign = NULL;
    
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

    case QP_SOCKET_DOMAIN_PACKET:{
        assign = qp_socket_assign_packet(skt);
    }break;

    default: {
        QP_LOGOUT_ERROR("[qp_socket_t] Socket not support.");
    }break;
    }
    
    if (assign) {
        return assign;
    }
    
    qp_socket_destroy(skt);
    return NULL;
}

qp_int_t
qp_socket_destroy(qp_socket_t* skt)
{
    if (qp_fd_is_inited(&skt->socket)) {
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

qp_socket_t*
qp_socket_assign_inet(qp_socket_t* skt, const qp_char_t* name, qp_ushort_t port) 
{
    if (!skt || !name || (0 == port)) {
        return NULL;
    }
    
    memset(&skt->socket_addr, 0, sizeof(struct sockaddr_in));
        skt->socket_addr.inet_addr.sin_family = skt->domain;

    /* listen */
    if (qp_socket_is_listen(skt)) {

        if (0 == strcmp(name, "0.0.0.0")) {
            skt->socket_addr.inet_addr.sin_addr.s_addr = INADDR_ANY;
            
        } else {
            skt->socket.retsno = inet_pton(skt->domain, name, 
                &skt->socket_addr.inet_addr.sin_addr);
            skt->socket.errono = errno;
                
            if (1 != skt->socket.retsno) {
//                qp_socket_destroy(skt);
                QP_LOGOUT_ERROR("[qp_socket_t] Ip switch fail.");
                return NULL;
            }
        }

        skt->socket_len = sizeof(struct sockaddr_in);
        skt->socket_addr.inet_addr.sin_port = ntohs(port);

        /* no listen */
    } else {
        skt->socket.retsno = inet_pton(skt->domain, \
                name,&skt->socket_addr.inet_addr.sin_addr);
        skt->socket.errono = errno;
            
        if (1 != skt->socket.retsno) {
//            qp_socket_destroy(skt);
            QP_LOGOUT_ERROR("[qp_socket_t] Ip switch fail.");
            return NULL;
        }

        skt->socket_len = sizeof(struct sockaddr_in);
        skt->socket_addr.inet_addr.sin_port = htons(port);
    }

    return skt;
}

qp_socket_t*
qp_socket_assign_inet6(qp_socket_t* skt, const qp_char_t* name, qp_ushort_t port) 
{
    if (!skt || !name || (0 == port)) {
        return NULL;
    }
    
    skt = NULL;
    name = NULL;
    return NULL;
}

qp_socket_t
qp_socket_assign_unix(qp_socket_t* skt, const qp_char_t* name) 
{
    if (!skt || !name) {
        return NULL;
    }
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
        len = offsetof(struct sockaddr_un, sun_path) \
            + strlen(saddr.sun_path);

        /* delete exit file */
        unlink(saddr.sun_path);

        /* bind scoket */
        skt->socket.retsno = bind(skt->socket.fd, \
            (struct sockaddr *)&saddr, len);
        skt->socket.errono = errno;
            
        if (QP_SUCCESS != skt->socket.retsno) {
//            qp_socket_destroy(skt);
            QP_LOGOUT_ERROR("[qp_socket_t] Unix socket bind error.");
            return NULL;
        }

        /* owner only */
        if (QP_SUCCESS != chmod(saddr.sun_path, S_IRWXU)) {
            unlink(saddr.sun_path);
//            qp_socket_destroy(skt);
            QP_LOGOUT_ERROR("[qp_socket_t] Unix socket priv set fail.");
            return NULL;
        }

        /* set server descriptor */
        strcpy(skt->socket_addr.unet_addr.sun_path, name);
        skt->socket_len = offsetof(struct sockaddr_un, sun_path) \
            + strlen(name);
    }

    return skt;
}

qp_socket_t* 
qp_socket_assign_packet(qp_socket_t* skt) 
{
    if (!skt) {
        return NULL
    }
    
    return NULL;
}

qp_int_t
qp_socket_listen(qp_socket_t* skt, qp_int_t mod)
{
    if (!qp_socket_is_listen(skt) || !qp_fd_is_valid(&skt->socket)) {
        return QP_ERROR;
    }

    skt->socket.retsno =  bind(skt->socket.fd, 
        (struct sockaddr*)&skt->socket_addr, skt->socket_len);
    skt->socket.errono = errno;
    
    if (QP_SUCCESS != skt->socket.retsno) {
        QP_LOGOUT_ERROR("[qp_socket_t] Socket bind fail.");
        return QP_ERROR;
    }

    skt->socket.retsno = listen(skt->socket.fd, skt->backlog);
    skt->socket.errono = errno;
    
    if (QP_SUCCESS != skt->socket.retsno) {
        QP_LOGOUT_ERROR("[qp_socket_t] Socket listen fail.");
        return QP_ERROR;
    }
    
    /* set rd/wr privilege */
    if (skt->domain == QP_SOCKET_DOMAIN_UNIX) {
        chmod(skt->socket_addr.unet_addr.sun_path, mod);
    }

    return QP_SUCCESS;
}

qp_int_t
qp_socket_setsockopt(qp_socket_t* skt, qp_int_t level, qp_int_t optname, \
    const void* optval, socklen_t optlen)
{
    if (!qp_fd_is_valid(&skt->socket)) {
        return QP_ERROR;
    }
    
    skt->socket.retsno = setsockopt(skt->socket.fd, level, optname, optval, 
        optlen);
    skt->socket.errono = errno;
    return skt->socket.retsno;
}


qp_int_t
qp_socket_close(qp_socket_t* skt, qp_socket_shut_t shut)
{
    if (qp_fd_is_valid(&skt->socket)) {
        
        /* close socket at once */
        if (shut != QP_SOCKET_SHUT_CLOSE) {
            shutdown(skt->socket.fd, shut);
        }

        return qp_fd_close(&skt->socket);
    }
    
    return QP_ERROR;
}

qp_socket_t*
qp_socket_accept(qp_socket_t* skt, qp_socket_t* sktClient)
{
    if (qp_socket_is_listen(skt)) {
        
        if (NULL == (sktClient = qp_socket_create(sktClient))) {
            QP_LOGOUT_ERROR("[qp_socket_t] Client create fail.");
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

        case QP_SOCKET_DOMAIN_PACKET:
            
        default:{
            sktClient->socket_len = sizeof(struct sockaddr);
        }break;
        }

        sktClient->socket.fd = accept(skt->socket.fd, \
            (struct sockaddr*)&sktClient->socket_addr, &sktClient->socket_len);
        skt->socket.errono = errno;
        return sktClient;
    }
    
    return NULL;
}

qp_int_t
qp_socket_connect(qp_socket_t* skt)
{
    if (qp_socket_is_listen(skt)) {
        return QP_ERROR;
    }

    skt->socket.retsno = connect(skt->socket.fd,
        (struct sockaddr*)&(skt->socket_addr), skt->socket_len);
    skt->socket.errono = errno;
    return skt->socket.retsno;
}

size_t
qp_socket_sendn(qp_socket_t* skt, const void* vptr, size_t nbytes)
{
    skt ? { return qp_fd_writen(&skt->socket, vptr, nbytes);} : \
        {return 0;};
}

size_t
qp_socket_recvn(qp_socket_t* skt, void* vptr, size_t nbytes)
{
    skt ? { return qp_fd_readn(&(skt->socket), vptr, nbytes)} :
        {return 0};
}

ssize_t
qp_socket_sendv(qp_socket_t* skt, const struct iovec* iov, qp_int_t iovcnt)
{
    skt ? { return qp_fd_writev(&(skt->socket), iov, iovcnt);} :
        {return QP_ERROR;};
}

ssize_t
qp_socket_recvv(qp_socket_t* skt, const struct iovec* iov, qp_int_t iovcnt)
{
    skt ? { return qp_fd_readv(&(skt->socket), iov, iovcnt);} :
        {return QP_ERROR;};
}

ssize_t
qp_socket_send(qp_socket_t* skt, const void* vptr, size_t nbytes, qp_int_t flag)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    skt->socket.retsno = send(skt->socket.fd, vptr, nbytes, flag);
    skt->socket.errono = errno;
    return skt->socket.retsno;
}

ssize_t
qp_socket_recv(qp_socket_t* skt, void* vptr, size_t nbytes, qp_int_t flag)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    skt->socket.retsno = recv(skt->socket.fd, vptr, nbytes, flag);
    skt->socket.errono = errno;
    return skt->socket.retsno;
}


/* option */
qp_int_t
qp_socket_set_reuse(qp_socket_t* skt, qp_int_t reuse, qp_int_t enable)
{
    if (reuse == QP_SOCKET_SO_REUSE_ADDR) {
        return qp_socket_setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, \
            (const void *)&enable, sizeof(enable));
    }
    
    if (reuse == QP_SOCKET_SO_REUSE_PORT) {
        return qp_socket_setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, \
            (const void *)&enable, sizeof(enable));
    }
    
    return QP_SUCCESS;
}

qp_int_t
qp_socket_set_nopush(qp_socket_t* skt, qp_int_t enable)
{   
#ifdef QP_OS_BSD4
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_NOPUSH, \
        (const void *)&enable, sizeof(enable));
#else
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_CORK, \
        (const void *)&enable, sizeof(enable));
#endif
}

qp_int_t
qp_socket_set_nodelay(qp_socket_t* skt, qp_int_t enable)
{
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, \
        (const void *)&enable, sizeof(enable));
}

qp_int_t
qp_socket_set_quickack(qp_socket_t* skt, qp_int_t enable)
{
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_QUICKACK, \
        (const void *)&enable, sizeof(enable));
}

