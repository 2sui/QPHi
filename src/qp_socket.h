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


#ifndef QP_SOCKET_H
#define QP_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"

    
# define  QP_SOCKET_DEFAULT_LISTENBACKLOG    128
# define  QP_SOCKET_DEFAULT_UNET_PATH        "/tmp/"
    
    
# define  QP_SOCKET_UNSUPPORT                QP_ERROR
    
    
enum qp_socket_shut_e {
    QP_SOCKET_SHUT_CLOSE = QP_ERROR,
    QP_SOCKET_SHUT_RD = SHUT_RD,
    QP_SOCKET_SHUT_WR = SHUT_WR,
    QP_SOCKET_SHUT_RDWR = SHUT_RDWR
};


enum qp_socket_domain_e {
    QP_SOCKET_DOMAIN_UNSUPPORT = QP_SOCKET_UNSUPPORT,
    QP_SOCKET_DOMAIN_UNIX = AF_UNIX,
    QP_SOCKET_DOMAIN_INET = AF_INET,
    QP_SOCKET_DOMAIN_INET6 = AF_INET6,
#if !defined(QP_OS_BSD)
    QP_SOCKET_DOMAIN_PACKET = AF_PACKET
#endif
};


enum qp_socket_type_e {
    QP_SOCKET_TYPE_UNSUPPORT = QP_SOCKET_UNSUPPORT,
    QP_SOCKET_TYPE_STREAM = SOCK_STREAM,
    QP_SOCKET_TYPE_DGRAM = SOCK_DGRAM,
    QP_SOCKET_TYPE_RAW = SOCK_RAW
};


typedef enum qp_socket_domain_e    qp_socket_domain_t;
typedef enum qp_socket_shut_e      qp_socket_shut_t;
typedef enum qp_socket_type_e      qp_socket_type_t;
typedef struct qp_socket_s*        qp_socket_t;


/**
 * Create a socket. 
 * If skt is NULL, it will allocate one for it and free the memory when calling 
 * qp_socket_destroy, and domain accept AF_UNIX, AF_INET... ,and type
 * accept SOCK_STREAM, SOCK_DGRAM... ,name is the addr string if using AF_INET 
 * or AF_INET6 or sock path using AF_UNIX.
 * 
 * @param skt: qp_socket_t struct or NULL.
 * @param domain: AF_INET, AF_INET6(Not support for now), AF_UNIX...
 * @param type: SOCK_STREAM, SOCK_DGRAM...
 * @param name: IP string for IP and IPv6 or path for unix socket.
 * @param port: Socket port for IP and IPv6 socket.
 * @param as_server: Run as socket server.
 * @param server_backlog: Backlog of socket server.
 * @return If success return pointer of skt, otherwise return NULL.
 */
qp_socket_t
qp_socket_init(qp_socket_t skt, qp_int_t domain, qp_int_t type, \
    const qp_char_t* name, qp_ushort_t port, \
    bool as_server, qp_int_t server_backlog);


qp_int_t
qp_socket_destroy(qp_socket_t skt);


/**
 * Get the [int] fd.
 */
qp_int_t
qp_socket_to_int(qp_socket_t skt);


qp_int_t
qp_socket_close(qp_socket_t skt, qp_socket_shut_t shut);


/**
 * Listen from created socket. [mod] is only work for unix socket for set the 
 * privilege of listening socket.
 * 
 * @param skt: Valid qp_socket_t.
 * @param mod: S_IRUSR,S_IWUSR,S_IXUSR...
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
qp_int_t
qp_socket_listen(qp_socket_t skt, qp_int_t mod);


/**
 * Accept client sockets. If sktClient is NULL it will allocate one
 * and you have to destroy it by call qp_socket_destroy.
 * 
 * @param skt: Valid qp_socket_t.
 * @param sktClient: Client qp_socket_t struct.If it is NULL the client socket 
 *     struct will be created by this method.
 * @return If success return addr of sktClient, otherwise return NULL.
 */
qp_socket_t
qp_socket_accept(qp_socket_t skt, qp_socket_t sktClient);


qp_int_t
qp_socket_connect(qp_socket_t skt);


/**
 * Set socket opt just like setsockopt.
 */
qp_int_t
qp_socket_setsockopt(qp_socket_t skt, qp_int_t level, qp_int_t optname, \
    const void* optval, socklen_t optlen);


/**
 * Send n bytes to socket.It will not break by signal.
 * 
 * @param skt: Valid qp_socket_t.
 * @param vptr: Send buffer.
 * @param nbytes: Send data size.
 * @return If success return value is equal to nbytes, otherwise return the size 
 *     that LESS than be nbytes.
 */
size_t
qp_socket_sendn(qp_socket_t skt, const void* vptr, size_t nbytes);


/**
 * Recv n bytes from socket.
 * 
 * @param skt: Valid qp_socket_t.
 * @param vptr: Recv buffer.
 * @param nbytes: Recv data size.
 * @return If success return value is equal to nbytes, otherwise return the size 
 *     that less than nbytes.(do not support TCP_QUICKACK).
 */
size_t
qp_socket_recvn(qp_socket_t skt, void* vptr, size_t nbytes);


/**
 * Just like sendv().
 * 
 * @param skt: Valid qp_socket_t.
 * @param iov: struct iovec pointer.
 * @param iovcnt: Size of struct iovec.
 * @return Return send data size.If some error happen return QP_ERROR, and 
 *     return 0 means peer closed.
 */
ssize_t
qp_socket_sendv(qp_socket_t skt, const struct iovec* iov, qp_int_t iovcnt);


/**
 * Just like recvv().
 * 
 * @param skt: Valid qp_socket_t.
 * @param iov: struct iovec pointer.
 * @param iovcnt: Size of struct iovec.
 * @return Return recv data size.If some error happen return QP_ERROR, and 
 *     return 0 means peer closed.
 */
ssize_t
qp_socket_recvv(qp_socket_t skt, const struct iovec* iov, qp_int_t iovcnt);

/*
 * Just like send().
*/
ssize_t
qp_socket_send(qp_socket_t skt, const void* vptr, size_t nbytes, qp_int_t flag);


/*
 * Just like recv().
*/
ssize_t
qp_socket_recv(qp_socket_t skt, void* vptr, size_t nbytes, qp_int_t flag);

/*
 * Set noblock.
 */
qp_int_t
qp_socket_set_noblock(qp_socket_t skt);

/* option */
# define QP_SOCKET_SO_REUSE_ADDR    SO_REUSEADDR
# define QP_SOCKET_SO_REUSE_PORT    SO_REUSEPORT

/* SO_REUSEADDR / SO_REUSEPORT */
qp_int_t
qp_socket_set_reuse(qp_socket_t skt, qp_int_t reuse, qp_int_t enable);


/* TCP_NOPUSH or TCP_CORK */
qp_int_t
qp_socket_set_nopush(qp_socket_t skt, qp_int_t enable);


/* TCP_NODELAY */
qp_int_t
qp_socket_set_nodelay(qp_socket_t skt, qp_int_t enable);


/* TCP_QUICKACK */
#if !defined(QP_OS_BSD)
qp_int_t
qp_socket_set_quickack(qp_socket_t skt, qp_int_t enable);
#endif

#ifdef __cplusplus
}
#endif

#endif /* QP_SOCKET_H */
