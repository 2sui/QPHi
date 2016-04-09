
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_io.h"


/*
 * Every fd struct has 4 stat: 1 bit for init stat, 1 bit for noblock stat,
 * 1 bit for async stat, 1 bit for alloced stat.
*/
inline void
qp_fd_set_inited(qp_fd_t* fd)
{ fd ? fd->is_inited = true: 1;}

inline void
qp_fd_set_alloced(qp_fd_t* fd)
{ fd ? fd->is_alloced = true : 1;}

inline void
qp_fd_set_noblock(qp_fd_t* fd)
{ fd ? fd->is_noblock = true : 1;}

//inline void
//qp_fd_set_async(qp_fd_t* fd)
//{ fd->is_async = true;}

inline void
qp_fd_unset_inited(qp_fd_t* fd)
{ fd ? fd->is_inited = false : 1;}

inline void
qp_fd_unset_alloced(qp_fd_t* fd)
{ fd ? fd->is_alloced = false : 1;}

inline void
qp_fd_unset_noblock(qp_fd_t* fd)
{ fd ? fd->is_noblock = false : 1;}

//inline void
//qp_fd_unset_async(qp_fd_t* fd)
//{ fd->is_async = false;}

inline bool
qp_fd_is_inited(qp_fd_t* fd) 
{ return fd ? fd->is_inited : false; }

inline bool
qp_fd_is_alloced(qp_fd_t* fd) 
{ return fd ? fd->is_alloced : false; }

inline bool
qp_fd_is_noblock(qp_fd_t* fd) 
{ return fd ? fd->is_noblock : false; }

//inline bool
//qp_fd_is_async(qp_fd_t* fd) 
//{ return fd->is_async; }

inline bool
qp_fd_is_valid(qp_fd_t* fd) 
{ return fd ? (QP_FD_INVALID != fd->fd) : false;}


qp_fd_t*
qp_fd_create(qp_fd_t* fd)
{
    if (NULL == fd) {
        fd = (qp_fd_t*)qp_alloc(sizeof(qp_fd_t));
        
        if (NULL == fd) {
            QP_LOGOUT_ERROR("[qp_io_t] FD create fail.");
            return NULL;
        }

        memset(fd, 0, sizeof(qp_fd_t));
        qp_fd_set_alloced(fd);

    } else {
        memset(fd, 0, sizeof(qp_fd_t));
    }

    fd->fd = QP_FD_INVALID;
    fd->type = QP_FD_TYPE_UNKNOW;
    qp_fd_set_inited(fd);
    return fd;
}

qp_fd_t*
qp_fd_init(qp_fd_t* fd, qp_fd_type_t type, bool aio)
{
    fd = qp_fd_create(fd);
    
    if (NULL == fd) {
        return NULL;
    }
    
    if (aio) {
        fd->aio = (struct aiocb*)qp_alloc(sizeof(struct aiocb));
        
//        if (fd->aio) {
//            qp_fd_set_async(fd);
//        }
    }

    fd->type = type;
    return fd;
}

qp_int_t
qp_fd_destroy(qp_fd_t *fd)
{
    if (qp_fd_is_inited(fd)) {
        qp_fd_close(fd);
        fd->type = QP_FD_TYPE_UNKNOW;
        
        if (fd->aio) {
            qp_free(fd->aio);
            fd->aio = NULL;
        }
        
        qp_fd_unset_noblock(fd);
//        qp_fd_unset_async(fd);
        qp_fd_unset_inited(fd);

        if (qp_fd_is_alloced(fd)) {
            qp_free(fd);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_fd_type_t
qp_fd_type(qp_fd_t *fd)
{
    if (qp_fd_is_inited(fd)) {
        return fd->type;
    }

    return QP_FD_TYPE_UNKNOW;
}

qp_int_t
qp_fd_setNoBlock(qp_fd_t* fd)
{
    if (!qp_fd_is_valid(fd) || fd->aio) {
        return QP_ERROR;
    }
    
    /*
     * check validity.qp_fd_is_noblock while return false if fd is NULL.So if 
     * return TRUE, fd is not NULL, otherwise we need to check it.
     */
    if (qp_fd_is_noblock(fd)) {
        return QP_SUCCESS;
    }

     fd->retsno = fcntl(fd->fd, F_SETFL, O_NONBLOCK | fcntl(fd->fd, F_GETFL));
     fd->errono = errno;
     
     if (QP_SUCCESS  == fd->retsno) {
         qp_fd_set_noblock(fd);
     }

    return fd->retsno;
}

qp_int_t
qp_fd_setBlock(qp_fd_t *fd)
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

    fd->retsno = fcntl(fd->fd, F_SETFL, (~O_NONBLOCK) & fcntl(fd->fd,F_GETFL));
    fd->errono = errno;
    
    if (QP_SUCCESS == fd->retsno) {
        qp_fd_unset_noblock(fd);
    }

    return fd->retsno;
}

/**
  * Use carefully!
*/

qp_int_t
qp_fd_close(qp_fd_t* fd)
{
    if (qp_fd_is_valid(fd)) {
        fd->retsno = close(fd->fd);
        fd->errono = errno;
        fd->fd = QP_FD_INVALID;
        return fd->retsno;
    }

    return QP_ERROR;
}

ssize_t
qp_fd_write(qp_fd_t* fd, const void* vptr, size_t nbytes)
{
    if (!fd) {
        return QP_ERROR;
    }
    
    fd->retsno = write(fd->fd, vptr, nbytes);
    fd->errono = errno;
    return fd->retsno;
}

ssize_t
qp_fd_read(qp_fd_t* fd, void* vptr, size_t nbytes)
{
    if (!fd) {
        return QP_ERROR;
    }
    
    fd->retsno = read(fd->fd, vptr, nbytes);
    fd->errono = errno;
    return fd->retsno;
}

size_t
qp_fd_writen(qp_fd_t *fd, const void *vptr, size_t nbytes)
{
    if (!fd) {
        return 0;
    }
    
    size_t  ndone = 0;
    
    while (ndone < nbytes) {
        fd->retsno = write(fd->fd, vptr, nbytes - ndone);
        fd->errono = errno;

        if (1 > fd->retsno) {

            if ((fd->retsno < 0) \
               && ((EINTR == fd->errono) || (EAGAIN == fd->errono) \
               || (EWOULDBLOCK == fd->errono))) 
            {
                fd->retsno = 0; /* write again */

            } else {
                return ndone; /* error or peer connection closed (EOF) */
            }
        }

        ndone = ndone + fd->retsno;
        vptr = (char*)vptr + fd->retsno;
    }

    return ndone;
}

size_t
qp_fd_readn(qp_fd_t *fd, void *vptr, size_t nbytes)
{
    if (!fd) {
        return 0;
    }
    
    size_t ndone = 0;

    while (0 < nbytes) {
        fd->retsno = read(fd->fd, vptr, nbytes - ndone);
        fd->errono = errno;

        if (1 > fd->retsno) {

            if ((0 > fd->retsno) \
                && ((EINTR == fd->errono) || (EAGAIN == fd->errono) \
                    || (EWOULDBLOCK == fd->errono))) 
            {
                fd->retsno = 0; /* read again */

            } else {
                return ndone; /* error or peer connection closed (EOF) */
            }

        }

        ndone = ndone + fd->retsno;
        vptr = (char*)vptr + fd->retsno;
    }

    return ndone;
}

ssize_t
qp_fd_writev(qp_fd_t *fd, const struct iovec *iov, qp_int_t iovcnt)
{
    if (!fd) {
        return QP_ERROR;
    }
    
    fd->retsno = writev(fd->fd, iov, iovcnt);
    fd->errono = errno;
    return fd->retsno;
}

ssize_t
qp_fd_readv(qp_fd_t *fd, const struct iovec *iov, qp_int_t iovcnt)
{
    if (!fd) {
        return QP_ERROR;
    }
    
    fd->retsno = readv(fd->fd, iov, iovcnt);
    fd->errono = errno;
    return fd->retsno;
}


qp_int_t
qp_fd_aio_sync(qp_fd_t* fd)
{
    if (!fd) {
        return QP_ERROR;
    }
    
//    if (qp_fd_is_async(fd)) {
    if (fd->aio) {
        fd->aio->aio_fildes = fd->fd;
//        fd->retsno = aio_fsync(O_DSYNC, &(fd->fd.aio));
        fd->retsno = aio_fsync(O_DSYNC, fd->aio);
        fd->errono = errno;
        return fd->retsno;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_fd_aio_stat(qp_fd_t* fd)
{
    if (!fd) {
        return QP_ERROR;
    }
    
//    if (qp_fd_is_async(fd)) {
    if (fd->aio) {
        fd->aio->aio_fildes = fd->fd;
//        fd->retsno = aio_error(&(fd->fd.aio));
        fd->retsno = aio_error(fd->aio);
        fd->errono = errno;
        return fd->retsno;
    }
    
    return QP_ERROR;
}

ssize_t
qp_fd_aio_write(qp_fd_t* fd, const void* vptr, size_t nbytes, size_t offset)
{
    if (!fd) {
        return QP_ERROR;
    }
    
    if (!fd->aio) {
        return 0;
    }
    
    fd->aio->aio_fildes = fd->fd;
//    fd->fd.aio.aio_buf = (void*)vptr;
//    fd->fd.aio.aio_nbytes = nbytes;
//    fd->fd.aio.aio_offset = offset;
//    fd->retsno = aio_write(&(fd->fd.aio));
    fd->aio->aio_buf = (void*)vptr;
    fd->aio->aio_nbytes = nbytes;
    fd->aio->aio_offset = offset;
    fd->retsno = aio_write(fd->aio);
    fd->errono = errno;
    return fd->retsno;
}

ssize_t
qp_fd_aio_read(qp_fd_t* fd, void* vptr, size_t nbytes, size_t offset)
{
    if (!fd) {
        return QP_ERROR;
    }
    
    if (!fd->aio) {
        return 0;
    }
    
    fd->aio->aio_fildes = fd->fd;
//    fd->fd.aio.aio_buf = vptr;
//    fd->fd.aio.aio_nbytes = nbytes;
//    fd->fd.aio.aio_offset = offset;
//    fd->retsno = aio_read(&(fd->fd.aio));
    fd->aio->aio_buf = vptr;
    fd->aio->aio_nbytes = nbytes;
    fd->aio->aio_offset = offset;
    fd->retsno = aio_read(fd->aio);
    fd->errono = errno;
    return fd->retsno;
}

