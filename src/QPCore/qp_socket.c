
/**
  * Copyright (C) 2sui.
  */


#include "qp_socket.h"
#include "qp_o_io.h"
#include "qp_o_memory.h"


struct  qp_socket_s {
    qp_fd_t                       socket;      /* fd */
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

inline void
qp_socket_set_alloced(qp_socket_t skt)
{ skt ? skt->is_alloced = true : 1;}

inline void
qp_socket_set_listen(qp_socket_t skt)
{ skt ? skt->is_listen = true : 1;}

inline void
qp_socket_unset_alloced(qp_socket_t skt)
{ skt ? skt->is_alloced = false : 1;}

inline void
qp_socket_unset_listen(qp_socket_t skt)
{ skt ? skt->is_listen = false : 1;}


inline bool
qp_socket_is_alloced(qp_socket_t skt) 
{ return skt ? skt->is_alloced : false; }

inline bool
qp_socket_is_listen(qp_socket_t skt) 
{ return skt ? skt->is_listen : false; }


// MARK: private functions.

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
    if (NULL == (skt->socket = qp_fd_init(skt->socket, QP_FD_TYPE_SOCKET, false))) {

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
    const qp_char_t* name, qp_ushort_t port, bool as_server, qp_int_t server_backlog)
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

    case QP_SOCKET_DOMAIN_PACKET: {
        return NULL; // NOT SUPPORTED
    }break;

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
    qp_fd_set_fd(skt->socket, socket(skt->domain, skt->type, 0));

    if (!qp_fd_is_valid(skt->socket)) {
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

    case QP_SOCKET_DOMAIN_PACKET:{
        assign = qp_socket_assign_packet(skt);
    }break;

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
    if (skt && qp_fd_is_inited(skt->socket)) {
        qp_socket_close(skt, QP_SOCKET_SHUT_CLOSE);
        qp_fd_destroy(skt->socket);
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
    if (!skt || !name || (0 == port)) {
        return NULL;
    }
    
    skt = NULL;
    name = NULL;
    return NULL;
}

qp_socket_t
qp_socket_assign_unix(qp_socket_t skt, const qp_char_t* name) 
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
        len = offsetof(struct sockaddr_un, sun_path) + strlen(saddr.sun_path);

        /* delete exit file */
        unlink(saddr.sun_path);

        /* bind scoket */
        if (QP_SUCCESS != bind(qp_fd_get_fd(skt->socket), (struct sockaddr*)&saddr, len)) {
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

qp_socket_t 
qp_socket_assign_packet(qp_socket_t skt) 
{
    if (!skt) {
        return NULL;
    }
    
    /* NOT SUPPORTED */
    return NULL;
}

qp_int_t
qp_socket_close(qp_socket_t skt, qp_socket_shut_t shut)
{
    if (skt && qp_fd_is_valid(skt->socket)) {
        
        /* close socket at once */
        if (shut != QP_SOCKET_SHUT_CLOSE) {
            shutdown(qp_fd_get_fd(skt->socket), shut);
        }

        return qp_fd_close(skt->socket);
    }
    
    return QP_ERROR;
}

qp_int_t
qp_socket_listen(qp_socket_t skt, qp_int_t mod)
{
    if (!qp_socket_is_listen(skt) || !qp_fd_is_valid(skt->socket)) {
        return QP_ERROR;
    }
    
    if (QP_SUCCESS != bind(qp_fd_get_fd(skt->socket), \
        (struct sockaddr*)&skt->socket_addr, skt->socket_len)) {
        return QP_ERROR;
    }
    
    if (QP_SUCCESS != listen(qp_fd_get_fd(skt->socket), skt->backlog)) {
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
    if (qp_socket_is_listen(skt) && qp_fd_is_valid(skt->socket)) {
        
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

        case QP_SOCKET_DOMAIN_PACKET:
            
        default:{
            sktClient->socket_len = sizeof(struct sockaddr);
        }break;
        }

        qp_fd_set_fd(sktClient->socket, accept(qp_fd_get_fd(skt->socket), \
            (struct sockaddr*)&sktClient->socket_addr, &sktClient->socket_len));
        return sktClient;
    }
    
    return NULL;
}

qp_int_t
qp_socket_connect(qp_socket_t skt)
{
    if (qp_socket_is_listen(skt) || !qp_fd_is_valid(skt->socket)) {
        return QP_ERROR;
    }

    return connect(qp_fd_get_fd(skt->socket), (struct sockaddr*)&(skt->socket_addr), \
        skt->socket_len);
}

qp_int_t
qp_socket_setsockopt(qp_socket_t skt, qp_int_t level, qp_int_t optname, \
    const void* optval, socklen_t optlen)
{
    if (!skt || !qp_fd_is_valid(skt->socket)) {
        return QP_ERROR;
    }
    
    return setsockopt(qp_fd_get_fd(skt->socket), level, optname, optval, optlen);
}

size_t
qp_socket_sendn(qp_socket_t skt, const void* vptr, size_t nbytes)
{
    return skt ? qp_fd_writen(skt->socket, vptr, nbytes) : 0;
}

size_t
qp_socket_recvn(qp_socket_t skt, void* vptr, size_t nbytes)
{
    return skt ? qp_fd_readn(skt->socket, vptr, nbytes) : 0;
}

ssize_t
qp_socket_sendv(qp_socket_t skt, const struct iovec* iov, qp_int_t iovcnt)
{
    return skt ? qp_fd_writev(skt->socket, iov, iovcnt) : QP_ERROR;
}

ssize_t
qp_socket_recvv(qp_socket_t skt, const struct iovec* iov, qp_int_t iovcnt)
{
    return skt ? qp_fd_readv(skt->socket, iov, iovcnt) : QP_ERROR;
}

ssize_t
qp_socket_send(qp_socket_t skt, const void* vptr, size_t nbytes, qp_int_t flag)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    return send(qp_fd_get_fd(skt->socket), vptr, nbytes, flag);
}

ssize_t
qp_socket_recv(qp_socket_t skt, void* vptr, size_t nbytes, qp_int_t flag)
{
    if (!skt) {
        return QP_ERROR;
    }
    
    return recv(qp_fd_get_fd(skt->socket), vptr, nbytes, flag);
}


/* option */
qp_int_t
qp_socket_set_reuse(qp_socket_t skt, qp_int_t reuse, qp_int_t enable)
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
qp_socket_set_nopush(qp_socket_t skt, qp_int_t enable)
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
qp_socket_set_nodelay(qp_socket_t skt, qp_int_t enable)
{
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, \
        (const void *)&enable, sizeof(enable));
}

qp_int_t
qp_socket_set_quickack(qp_socket_t skt, qp_int_t enable)
{
    return qp_socket_setsockopt(skt, IPPROTO_TCP, TCP_QUICKACK, \
        (const void *)&enable, sizeof(enable));
}
