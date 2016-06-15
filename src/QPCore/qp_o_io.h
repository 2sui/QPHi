
/**
 * Copyright (C) 2sui.
 *
*/


#ifndef QP_O_IO_H
#define QP_O_IO_H


#include "qp_o_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define  QP_FD_INVALID        QP_ERROR
    
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


bool
qp_fd_is_inited(qp_fd_t fd);

bool
qp_fd_is_alloced(qp_fd_t fd) ;

bool
qp_fd_is_noblock(qp_fd_t fd);

bool
qp_fd_is_aio(qp_fd_t fd);

bool
qp_fd_is_valid(qp_fd_t fd);

/**
 * Init a qp_fd_t struct with specific type.If the fd is NULL it will allocate 
 * a qp_fd_t struct and init it.
 * 
 * @param fd:  An qp_fd_t pointer.If it is NULL this method will allocate one and 
 *     init it.
 * @param type: Type of qp_fd_t, it must be one of QP_FD_TYPE_FILE, 
 *     QP_FD_TYPE_SOCKET and QP_FD_TYPE_EVENT.
 * @param aio: Should be set as AIO mod. 
 * @return  If success return qp_fd_t pointer,  otherwise return NULL.
 */
qp_fd_t 
qp_fd_init(qp_fd_t fd, qp_fd_type_t type, bool aio);

/**
 * Destroy a fd. If the fd is allocated by qp_fd_create, it will be freed.
 * 
 * @param fd: Inited qp_fd_t.
 * @return QP_SUCCESS if success, otherwise return QP_ERROR.
 */
qp_int_t
qp_fd_destroy(qp_fd_t fd);

/**
 * Get the type of fd.
 * 
 * @param fd: Inited qp_fd_t.
 * @return Type of qp_fd_t if success, otherwise return QP_FD_TYPE_UNKNOW.
 */
qp_fd_type_t
qp_fd_type(qp_fd_t fd);

/**
 * Set the fd with NOBLOCK mod.
 * 
 * @param fd: Inited qp_fd_t.
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
qp_int_t
qp_fd_setNoBlock(qp_fd_t fd);

/**
 * Set the fd BLOCK mod.
 * 
 * @param fd: Inited qp_fd_t.
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
qp_int_t
qp_fd_setBlock(qp_fd_t fd);

/**
 * Set qp_fd_t value. And current errno will be set to qp_fd_t.
 * 
 * @param fd
 * @param ifd
 * @return 
 */
//qp_int_t
//qp_fd_set_fd(qp_fd_t fd, qp_int_t ifd);

/**
 * Get qp_fd_t value.
 * 
 * @param fd
 * @return 
 */
//qp_int_t
//qp_fd_get_fd(qp_fd_t fd);

/**
 * Close an opened fd.
 * 
 * @param fd: Close an opened qp_fd_t.
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
qp_int_t
qp_fd_close(qp_fd_t fd);

/**
 * Same with system write .
 * 
 * @param fd: Valid qp_fd_t.
 * @param vptr: Write buf.
 * @param nbytes: Write size.
 * @return Return writen size if success, and return 0 if file is closed,
 *     and return QP_ERROR if some error happend.
 */
ssize_t
qp_fd_write(qp_fd_t fd, const void* vptr, size_t nbytes);

/**
 * Same with system read.
 * 
 * @param fd: Valid qp_fd_t.
 * @param vptr: Read buf.
 * @param nbytes: Max read buf size.
 * @return Return read size if success, and return 0 if file is closed,
 *     and return QP_ERROR if some error happend.
 */
ssize_t
qp_fd_read(qp_fd_t fd, void* vptr, size_t nbytes);

/**
 * Write [nbytes] bytes to fd. 
 * 
 * @param fd: Valid qp_fd_t.
 * @param vptr: Write buf.
 * @param nbytes: Write buf size.
 * @return If success the return size is equal to [nbytes], otherwise some error 
 *     may happen, check fd->errono.
 */
size_t
qp_fd_writen(qp_fd_t fd, const void* vptr, size_t nbytes);

/**
 * Read [nbytes] bytes from fd. 
 * Return the number of read byte.
 * 
 * @param fd: Valid qp_fd_t.
 * @param vptr: Read buf.
 * @param nbytes: Max read buf size.
 * @return If success the return size is equal to [nbytes], otherwise some error
 *     may happen, check fd->errono.
 */
size_t
qp_fd_readn(qp_fd_t fd, void* vptr, size_t nbytes);

/**
 * Write vector to fd.
 * 
 * @param fd: Valid qp_fd_t.
 * @param iov: struct iovec pointer.
 * @param iovcnt: Size of iovec array.
 * @return Return writen size if success, and return 0 if file is closed,
 *     and return QP_ERROR if some error happend.
 */
ssize_t
qp_fd_writev(qp_fd_t fd, const struct iovec* iov, qp_int_t iovcnt);

/**
 * Read vector from fd.
 * 
 * @param fd: Valid qp_fd_t.
 * @param iov: struct iovec pointer.
 * @param iovcnt: Size of iovec array.
 * @return Return read size if success, and return 0 if file is closed,
 *     and return QP_ERROR if some error happend.
 */
ssize_t
qp_fd_readv(qp_fd_t fd, const struct iovec* iov, qp_int_t iovcnt);

/**
 * If enable aio, sync all data in queue to disk.
 * 
 * @param fd
 * @return 
 */
qp_int_t
qp_fd_aio_sync(qp_fd_t fd);

/**
 * Get aio stat of aio_read, aio_write or aio_sync.
 * 
 * @param fd
 * @return 
 */
qp_int_t
qp_fd_aio_stat(qp_fd_t fd);

/**
 * Same with aio_write.
 * 
 * @param fd
 * @param vptr
 * @param nbytes
 * @param offset
 * @return 
 */
ssize_t
qp_fd_aio_write(qp_fd_t fd, const void* vptr, size_t nbytes, size_t offset);

/**
 * Same with aio_read.
 * 
 * @param fd
 * @param vptr
 * @param nbytes
 * @param offset
 * @return 
 */
ssize_t
qp_fd_aio_read(qp_fd_t fd, void* vptr, size_t nbytes, size_t offset);

#ifdef __cplusplus
}
#endif

#endif 
