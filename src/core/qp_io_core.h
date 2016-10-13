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


#ifndef QP_IO_CORE_H
#define QP_IO_CORE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "qp_defines.h"    
    
    
# define  QP_FD_INVALID        QP_ERROR
    
    
enum qp_fd_type_e {
    QP_FD_TYPE_UNKNOW = 0,
    QP_FD_TYPE_FILE,
    QP_FD_TYPE_SOCKET,
    QP_FD_TYPE_EVENT
};


struct  qp_fd_s {
    qp_int_t          fd;
    enum qp_fd_type_e type;        /* type of this fd */
    struct aiocb*     aio;         /* fd */
    bool              is_inited;   /* is struct inited */
    bool              is_alloced;  /* is this fd allocated? */
    bool              is_noblock;  /* is fd noblock? */
};


typedef enum   qp_fd_type_e     qp_fd_type_t;
typedef struct qp_fd_s*         qp_fd_t;


static inline bool
qp_fd_is_inited(qp_fd_t fd)
{
    return fd->is_inited;
}


static inline bool
qp_fd_is_alloced(qp_fd_t fd)
{
    return fd->is_alloced;
}


static inline bool
qp_fd_is_noblock(qp_fd_t fd)
{
   return fd->is_noblock; 
}


static inline bool
qp_fd_is_aio(qp_fd_t fd)
{
    return NULL != fd->aio;
}


static inline bool
qp_fd_is_valid(qp_fd_t fd)
{
    return QP_FD_INVALID != fd->fd;
}


static inline qp_int_t
qp_fd_get_fd(qp_fd_t fd) 
{
    return fd->fd;
}


static inline qp_fd_type_t
qp_fd_type(qp_fd_t fd)
{
    return fd->type;
}


qp_fd_t 
qp_fd_init(qp_fd_t fd, qp_fd_type_t type, bool aio);


qp_int_t
qp_fd_destroy(qp_fd_t fd);


qp_int_t
qp_fd_setNoBlock(qp_fd_t fd);


qp_int_t
qp_fd_setBlock(qp_fd_t fd);


qp_int_t
qp_fd_close(qp_fd_t fd);


ssize_t
qp_fd_write(qp_fd_t fd, const void* vptr, size_t nbytes);


ssize_t
qp_fd_read(qp_fd_t fd, void* vptr, size_t nbytes);


size_t
qp_fd_writen(qp_fd_t fd, const void* vptr, size_t nbytes);


size_t
qp_fd_readn(qp_fd_t fd, void* vptr, size_t nbytes);


ssize_t
qp_fd_writev(qp_fd_t fd, const struct iovec* iov, qp_int_t iovcnt);


ssize_t
qp_fd_readv(qp_fd_t fd, const struct iovec* iov, qp_int_t iovcnt);


qp_int_t
qp_fd_aio_sync(qp_fd_t fd);


qp_int_t
qp_fd_aio_stat(qp_fd_t fd);


ssize_t
qp_fd_aio_write(qp_fd_t fd, const void* vptr, size_t nbytes, size_t offset);


ssize_t
qp_fd_aio_read(qp_fd_t fd, void* vptr, size_t nbytes, size_t offset);

#ifdef __cplusplus
}
#endif

#endif /* QP_IO_CORE_H */
