
/**
  * Copyright (C) 2sui.
  *
  * Basic socket operations.
  */


#ifndef QP_SOCKET_H
#define QP_SOCKET_H


#ifdef __cplusplus
extern "C" {
#endif


#include "qp_o_io.h"
#include "qp_tiny_stack.h"
    
    
#define  QP_SOCKET_DEFAULT_LISTENBACKLOG    128
#define  QP_SOCKET_DEFAULT_UNET_PATH        "/tmp/"


#define  QP_SOCKET_UNSUPPORT        -1
    
enum qp_socket_shut_e {
    QP_SOCKET_SHUT_CLOSE = -1,
    QP_SOCKET_SHUT_RD = SHUT_RD,
    QP_SOCKET_SHUT_WR = SHUT_WR,
    QP_SOCKET_SHUT_RDWR = SHUT_RDWR
};

typedef enum qp_socket_shut_e    qp_socket_shut_t;

enum qp_socket_domain_e {
    QP_SOCKET_DOMAIN_UNSUPPORT = QP_SOCKET_UNSUPPORT,
    QP_SOCKET_DOMAIN_UNIX = AF_UNIX,
    QP_SOCKET_DOMAIN_INET = AF_INET,
    QP_SOCKET_DOMAIN_INET6 = AF_INET6,
    QP_SOCKET_DOMAIN_PACKET = AF_PACKET
};

typedef enum qp_socket_domain_e    qp_socket_domain_t;
    

enum qp_socket_type_e {
    QP_SOCKET_TYPE_UNSUPPORT = QP_SOCKET_UNSUPPORT,
    QP_SOCKET_TYPE_STREAM = SOCK_STREAM,
    QP_SOCKET_TYPE_DGRAM = SOCK_DGRAM,
    QP_SOCKET_TYPE_RAW = SOCK_RAW
};

typedef enum qp_socket_type_e    qp_socket_type_t;


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

typedef struct qp_socket_s        qp_socket_t;


inline bool
qp_socket_is_alloced(qp_socket_t* skt);

inline bool
qp_socket_is_listen(qp_socket_t* skt);

/*
 * Create a socket. 
 * If skt is NULL, it will allocate one for it and free the memory when call 
 * qp_socket_destroy, and domain accept AF_UNIX, AF_INET... ,and type
 * accept SOCK_STREAM, SOCK_DGRAM... ,name is the addr string if using AF_INET 
 * or AF_INET6 or sock path using AF_UNIX.
 * If success return address of skt, otherwise return NULL.
 */
qp_socket_t*
qp_socket_init(qp_socket_t* skt, qp_int_t domain, qp_int_t type, 
    const qp_char_t* name, qp_ushort_t port, bool as_server);

/*
 * Destroy a socket.
 */
qp_int_t
qp_socket_destroy(qp_socket_t* skt);

/*
 * Listen from created socket.
 * [mod] is only work for unix socket for set the privilege of 
 * listening socket.
 * Return QP_SUCCESS if success otherwise return QP_ERROR.
*/
qp_int_t
qp_socket_listen(qp_socket_t* skt, qp_int_t mod);

/**
 * Set socket opt just like setsockopt.
 */
qp_int_t
qp_socket_setsockopt(qp_socket_t* skt, qp_int_t level, qp_int_t optname, \
    const void* optval, socklen_t optlen);

/*
 * Clsoe an opened socket.
*/
qp_int_t
qp_socket_close(qp_socket_t* skt, qp_socket_shut_t shut);

/*
 * Accept client sockets. If sktClient is NULL it will allocate one
 * and you have to destroy it by call qp_socket_destroy.
 * If success return addr of sktClient, otherwise return NULL.
*/
qp_socket_t*
qp_socket_accept(qp_socket_t* skt, qp_socket_t* sktClient);

/*
 * Connect to host
*/
qp_int_t
qp_socket_connect(qp_socket_t* skt);

/*
 * Send n bytes to socket.It will not break by signal.If success return value 
 * is equal to nbytes, otherwise return the size that LESS than be nbytes.
 */
size_t
qp_socket_sendn(qp_socket_t* skt, const void* vptr, size_t nbytes);

/*
 * Recv n bytes from socket.If success return value is equal to nbytes,
 * otherwise return the size that less than nbytes.(do not support TCP_QUICKACK)
*/
size_t
qp_socket_recvn(qp_socket_t* skt, void* vptr, size_t nbytes);


ssize_t
qp_socket_sendv(qp_socket_t* skt, const struct iovec* iov, qp_int_t iovcnt);

ssize_t
qp_socket_recvv(qp_socket_t* skt, const struct iovec* iov, qp_int_t iovcnt);

/*
 * Just like send().
*/
ssize_t
qp_socket_send(qp_socket_t* skt, const void* vptr, size_t nbytes, qp_int_t flag);

/*
 * Just like recv().
*/
ssize_t
qp_socket_recv(qp_socket_t* skt, void* vptr, size_t nbytes, qp_int_t flag);


/* option */
#define QP_SOCKET_SO_REUSE_ADDR    SO_REUSEADDR
#define QP_SOCKET_SO_REUSE_PORT    SO_REUSEPORT

/* SO_REUSEADDR / SO_REUSEPORT */
qp_int_t
qp_socket_set_reuse(qp_socket_t* skt, qp_int_t reuse, qp_int_t enable);

/* TCP_NOPUSH or TCP_CORK */
qp_int_t
qp_socket_set_nopush(qp_socket_t* skt, qp_int_t enable);

/* TCP_NODELAY */
qp_int_t
qp_socket_set_nodelay(qp_socket_t* skt, qp_int_t enable);

/* TCP_QUICKACK */
qp_int_t
qp_socket_set_quickack(qp_socket_t* skt, qp_int_t enable);


#ifdef __cplusplus
}
#endif

#endif 
