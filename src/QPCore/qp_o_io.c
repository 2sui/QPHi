
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_io.h"
#include "qp_o_memory.h"


inline void
qp_fd_set_inited(qp_fd_t fd)
{ fd ? fd->is_inited = true: 1;}

inline void
qp_fd_set_alloced(qp_fd_t fd)
{ fd ? fd->is_alloced = true : 1;}

inline void
qp_fd_set_noblock(qp_fd_t fd)
{ fd ? fd->is_noblock = true : 1;}

inline void
qp_fd_unset_inited(qp_fd_t fd)
{ fd ? fd->is_inited = false : 1;}

inline void
qp_fd_unset_alloced(qp_fd_t fd)
{ fd ? fd->is_alloced = false : 1;}

inline void
qp_fd_unset_noblock(qp_fd_t fd)
{ fd ? fd->is_noblock = false : 1;}

inline bool
qp_fd_is_inited(qp_fd_t fd) 
{ return fd ? fd->is_inited : false; }

inline bool
qp_fd_is_alloced(qp_fd_t fd) 
{ return fd ? fd->is_alloced : false; }

inline bool
qp_fd_is_noblock(qp_fd_t fd) 
{ return fd ? fd->is_noblock : false; }

inline bool
qp_fd_is_aio(qp_fd_t fd)
{ return fd ? (NULL != fd->aio) : false; }

inline bool
qp_fd_is_valid(qp_fd_t fd) 
{ return fd ? (QP_FD_INVALID != fd->fd) : false;}


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
    if (!qp_fd_is_valid(fd) || fd->aio) {
        return QP_ERROR;
    }
    
    /*
     * check validity.qp_fd_is_noblock while return false if the fd is NULL.So if 
     * returns TRUE, fd is not NULL, otherwise we need to check it.
     */
    if (qp_fd_is_noblock(fd)) {
        return QP_SUCCESS;
    }

    int ret = fcntl(fd->fd, F_SETFL, O_NONBLOCK | fcntl(fd->fd, F_GETFL));
     if (QP_SUCCESS == ret) {
         qp_fd_set_noblock(fd);
     }

    return ret;
}

qp_int_t
qp_fd_setBlock(qp_fd_t fd)
{
    if (!qp_fd_is_valid(fd) || fd->aio) {
        return QP_ERROR;
    }
    
    /*
     * check validity.qp_fd_is_noblock while return false if fd is NULL.So if 
     * return FALSE, fd may be NULL, and we need to check it.
    */
    if (!qp_fd_is_noblock(fd)) {
        return QP_SUCCESS;
    }

    int ret = fcntl(fd->fd, F_SETFL, (~O_NONBLOCK) & fcntl(fd->fd,F_GETFL));
    
    if (QP_SUCCESS == ret) {
        qp_fd_unset_noblock(fd);
    }

    return ret;
}

qp_int_t
qp_fd_set_fd(qp_fd_t fd, qp_int_t ifd) {
    if (qp_fd_is_inited(fd)) {
        fd->fd = ifd;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

inline qp_int_t
qp_fd_get_fd(qp_fd_t fd) {
    return qp_fd_is_inited(fd) ? fd->fd : QP_ERROR;
}

inline qp_fd_type_t
qp_fd_type(qp_fd_t fd)
{
    return qp_fd_is_inited(fd) ? fd->type : QP_FD_TYPE_UNKNOW;
}

qp_int_t
qp_fd_close(qp_fd_t fd)
{
    if (qp_fd_is_valid(fd)) {
        int ret = close(fd->fd);
        fd->fd = QP_FD_INVALID;
        return ret;
    }

    return QP_ERROR;
}

ssize_t
qp_fd_write(qp_fd_t fd, const void* vptr, size_t nbytes)
{
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    return write(fd->fd, vptr, nbytes);
}

ssize_t
qp_fd_read(qp_fd_t fd, void* vptr, size_t nbytes)
{
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    return read(fd->fd, vptr, nbytes);
}

size_t
qp_fd_writen(qp_fd_t fd, const void *vptr, size_t nbytes)
{
    if (!qp_fd_is_valid(fd)) {
        return 0;
    }
    
    size_t  ndone = 0;
    int     ret = 0;
    
    while (ndone < nbytes) {
        ret = write(fd->fd, vptr, nbytes - ndone);

        if (1 > ret) {

            if ((ret < 0) \
               && ((EINTR == errno) || (EAGAIN == errno) \
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
    if (!qp_fd_is_valid(fd)) {
        return 0;
    }
    
    size_t  ndone = 0;
    int     ret = 0;

    while (0 < nbytes) {
        ret = read(fd->fd, vptr, nbytes - ndone);

        if (1 > ret) {

            if ((0 > ret) \
                && ((EINTR == errno) || (EAGAIN == errno) \
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
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    return writev(fd->fd, iov, iovcnt);
}

ssize_t
qp_fd_readv(qp_fd_t fd, const struct iovec *iov, qp_int_t iovcnt)
{
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    return readv(fd->fd, iov, iovcnt);
}


qp_int_t
qp_fd_aio_sync(qp_fd_t fd)
{
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    if (fd->aio) {
        fd->aio->aio_fildes = fd->fd;
        return aio_fsync(O_DSYNC, fd->aio);
    }
    
    return QP_ERROR;
}

qp_int_t
qp_fd_aio_stat(qp_fd_t fd)
{
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    if (fd->aio) {
        fd->aio->aio_fildes = fd->fd;
        return aio_error(fd->aio);
    }
    
    return QP_ERROR;
}

ssize_t
qp_fd_aio_write(qp_fd_t fd, const void* vptr, size_t nbytes, size_t offset)
{
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
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
    if (!qp_fd_is_valid(fd)) {
        return QP_ERROR;
    }
    
    if (!fd->aio) {
        return 0;
    }
    
    fd->aio->aio_fildes = fd->fd;
    fd->aio->aio_buf = vptr;
    fd->aio->aio_nbytes = nbytes;
    fd->aio->aio_offset = offset;
    return aio_read(fd->aio);
}
