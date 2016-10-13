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


#include "qp_io_core.h"
#include "qp_memory_core.h"


static inline void
qp_fd_set_inited(qp_fd_t fd)
{ 
    fd->is_inited = true;
}


static inline void
qp_fd_set_alloced(qp_fd_t fd)
{ 
    fd->is_alloced = true;
}


static inline void
qp_fd_set_noblock(qp_fd_t fd)
{ 
    fd->is_noblock = true;
}


static inline void
qp_fd_unset_inited(qp_fd_t fd)
{ 
    fd->is_inited = false;
}


static inline void
qp_fd_unset_noblock(qp_fd_t fd)
{ 
    fd->is_noblock = false;
}


qp_fd_t
qp_fd_create(qp_fd_t fd)
{
    if (NULL == fd) {
        fd = (qp_fd_t)qp_alloc(sizeof(struct qp_fd_s));
        
        if (NULL == fd) {
            return NULL;
        }

        memset(fd, 0, sizeof(struct qp_fd_s));
        qp_fd_set_alloced(fd);

    } else {
        memset(fd, 0, sizeof(struct qp_fd_s));
    }

    fd->fd = QP_FD_INVALID;
    fd->type = QP_FD_TYPE_UNKNOW;
    qp_fd_set_inited(fd);
    return fd;
}


qp_fd_t
qp_fd_init(qp_fd_t fd, qp_fd_type_t type, bool aio)
{
    fd = qp_fd_create(fd);
    
    if (NULL == fd) {
        return NULL;
    }
    
    if (aio) {
        fd->aio = (struct aiocb*)qp_alloc(sizeof(struct aiocb));
    }

    fd->type = type;
    return fd;
}


qp_int_t
qp_fd_destroy(qp_fd_t fd)
{
    if (qp_fd_is_inited(fd)) {
        qp_fd_close(fd);
        fd->type = QP_FD_TYPE_UNKNOW;
        
        if (fd->aio) {
            qp_free(fd->aio);
            fd->aio = NULL;
        }
        
        qp_fd_unset_noblock(fd);
        qp_fd_unset_inited(fd);

        if (qp_fd_is_alloced(fd)) {
            qp_free(fd);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


qp_int_t
qp_fd_setNoBlock(qp_fd_t fd)
{
    if (fd->aio) {
        return QP_ERROR;
    }
    
    /*
     * check validity.qp_fd_is_noblock while return false if the fd is NULL.So if 
     * returns TRUE, fd is not NULL, otherwise we need to check it.
     */
    if (qp_fd_is_noblock(fd)) {
        return QP_SUCCESS;
    }

    if (QP_SUCCESS == fcntl(fd->fd,F_SETFL,O_NONBLOCK|fcntl(fd->fd, F_GETFL))) {
        qp_fd_set_noblock(fd);
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


qp_int_t
qp_fd_setBlock(qp_fd_t fd)
{
    if (fd->aio) {
        return QP_ERROR;
    }
    
    /*
     * check validity.qp_fd_is_noblock while return false if fd is NULL.So if 
     * return FALSE, fd may be NULL, and we need to check it.
    */
    if (!qp_fd_is_noblock(fd)) {
        return QP_SUCCESS;
    }

    if (QP_SUCCESS == fcntl(fd->fd,F_SETFL,(~O_NONBLOCK)&fcntl(fd->fd,F_GETFL))) {
        qp_fd_unset_noblock(fd);
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


qp_int_t
qp_fd_close(qp_fd_t fd)
{
    if (qp_fd_is_valid(fd)) {
        qp_int_t ret = fd->fd;
        fd->fd = QP_FD_INVALID;
        return close(ret);
    }

    return QP_ERROR;
}


ssize_t
qp_fd_write(qp_fd_t fd, const void* vptr, size_t nbytes)
{
    return write(fd->fd, vptr, nbytes);
}


ssize_t
qp_fd_read(qp_fd_t fd, void* vptr, size_t nbytes)
{
    return read(fd->fd, vptr, nbytes);
}


size_t
qp_fd_writen(qp_fd_t fd, const void *vptr, size_t nbytes)
{
    size_t  ndone = 0;
    int     ret = 0;
    
    while (ndone < nbytes) {
        ret = write(fd->fd, vptr, nbytes - ndone);

        if (1 > ret) {

            if ((ret < 0) && ((EINTR == errno) || (EAGAIN == errno) \
               || (EWOULDBLOCK == errno))) 
            {
                ret = 0; /* write again */

            } else {
                return ndone; /* error or peer connection closed (EOF) */
            }
        }

        ndone = ndone + ret;
        vptr = (char*)vptr + ret;
    }

    return ndone;
}


size_t
qp_fd_readn(qp_fd_t fd, void *vptr, size_t nbytes)
{
    size_t  ndone = 0;
    int     ret = 0;

    while (0 < nbytes) {
        ret = read(fd->fd, vptr, nbytes - ndone);

        if (1 > ret) {

            if ((0 > ret) && ((EINTR == errno) || (EAGAIN == errno) \
                || (EWOULDBLOCK == errno))) 
            {
                ret = 0; /* read again */

            } else {
                return ndone; /* error or peer connection closed (EOF) */
            }

        }

        ndone = ndone + ret;
        vptr = (char*)vptr + ret;
    }

    return ndone;
}


ssize_t
qp_fd_writev(qp_fd_t fd, const struct iovec *iov, qp_int_t iovcnt)
{
    return writev(fd->fd, iov, iovcnt);
}


ssize_t
qp_fd_readv(qp_fd_t fd, const struct iovec *iov, qp_int_t iovcnt)
{
    return readv(fd->fd, iov, iovcnt);
}


qp_int_t
qp_fd_aio_sync(qp_fd_t fd)
{
    if (fd->aio) {
        fd->aio->aio_fildes = fd->fd;
        return aio_fsync(O_DSYNC, fd->aio);
    }
    
    return QP_ERROR;
}


qp_int_t
qp_fd_aio_stat(qp_fd_t fd)
{
    if (fd->aio) {
        fd->aio->aio_fildes = fd->fd;
        return aio_error(fd->aio);
    }
    
    return QP_ERROR;
}


ssize_t
qp_fd_aio_write(qp_fd_t fd, const void* vptr, size_t nbytes, size_t offset)
{
    if (!fd->aio) {
        return 0;
    }
    
    fd->aio->aio_fildes = fd->fd;
    fd->aio->aio_buf = (void*)vptr;
    fd->aio->aio_nbytes = nbytes;
    fd->aio->aio_offset = offset;
    return aio_write(fd->aio);
}


ssize_t
qp_fd_aio_read(qp_fd_t fd, void* vptr, size_t nbytes, size_t offset)
{
    if (!fd->aio) {
        return 0;
    }
    
    fd->aio->aio_fildes = fd->fd;
    fd->aio->aio_buf = vptr;
    fd->aio->aio_nbytes = nbytes;
    fd->aio->aio_offset = offset;
    return aio_read(fd->aio);
}
