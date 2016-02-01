
/**
 *  Copyright (C) sui
 *
 * Basic I/O opeartions.
*/


#ifndef QP_O_IO_H
#define QP_O_IO_H


#ifdef __cplusplus
extern "C" {
#endif


#include "qp_o_memory.h"
    
    
#define  QP_FD_INVALID        -1
    
enum qp_fd_type_e {
    QP_FD_TYPE_UNKNOW = 0,
    QP_FD_TYPE_FILE,
    QP_FD_TYPE_SOCKET,
    QP_FD_TYPE_EVENT
};

typedef enum qp_fd_type_e    qp_fd_type_t;


struct  qp_fd_s {
    qp_int_t          fd;
    qp_fd_type_t      type;        /* type of this fd */
    struct aiocb*     aio;         /* fd */
    ssize_t           retsno;      /* return value (errno) */
    qp_int_t          errono;      /* error value */
    bool              is_inited;   /* is struct inited */
    bool              is_alloced;  /* is this fd allocated? */
    bool              is_noblock;  /* is fd noblock? */
//    bool              is_async;    /* is async(not used) */
};

typedef  struct qp_fd_s      qp_fd_t;


/**
  * Check validity.
*/
inline bool
qp_fd_is_inited(qp_fd_t* fd);

inline bool
qp_fd_is_alloced(qp_fd_t* fd) ;

inline bool
qp_fd_is_noblock(qp_fd_t* fd);

//inline bool
//qp_fd_is_async(qp_fd_t* fd);

inline bool
qp_fd_is_valid(qp_fd_t* fd);

/*
 * Init a qp_fd_t struct with specific type.
 * If the fd is NULL it will allocate a struct qp_fd_t and set it.
 * If success return qp_fd_t pointer,  otherwise return NULL.
*/
qp_fd_t*
qp_fd_init(qp_fd_t* fd, qp_fd_type_t type, bool aio);

/*
 * Destroy a fd.
 * If the fd is allocated by qp_fd_create, it will be freed.
*/
qp_int_t
qp_fd_destroy(qp_fd_t* fd);

/*
 * Get the type of fd.
*/
qp_fd_type_t
qp_fd_type(qp_fd_t* fd);

/*
 * Set the fd no block.
*/
qp_int_t
qp_fd_setNoBlock(qp_fd_t* fd);

/*
 * Set the fd block.
*/
qp_int_t
qp_fd_setBlock(qp_fd_t* fd);

/*
 * Close an opened fd.
*/
qp_int_t
qp_fd_close(qp_fd_t* fd);

/* Same with system write */
ssize_t
qp_fd_write(qp_fd_t* fd, const void* vptr, size_t nbytes);

/* Same with system read */
ssize_t
qp_fd_read(qp_fd_t* fd, void* vptr, size_t nbytes);

/*
 * Write [nbytes] bytes to fd. 
 * Return the number of writen byte.
 * If error happend return size less than [nbytes].
 */
size_t
qp_fd_writen(qp_fd_t* fd, const void* vptr, size_t nbytes);

/*
 * Read [nbytes] bytes from fd. 
 * Return the number of read byte.
 * If error happend return size less than [nbytes].
 */
size_t
qp_fd_readn(qp_fd_t* fd, void* vptr, size_t nbytes);

/**
 * Write vector to fd.
 */
ssize_t
qp_fd_writev(qp_fd_t* fd, const struct iovec* iov, qp_int_t iovcnt);

/**
 * Read vector from fd.
 */
ssize_t
qp_fd_readv(qp_fd_t* fd, const struct iovec* iov, qp_int_t iovcnt);

/**
 * If enable aio, sync all data in queue to disk.
 */
qp_int_t
qp_fd_aio_sync(qp_fd_t* fd);

/**
 * Get aio stat of aio_read aio_write or aio_sync.
 */
qp_int_t
qp_fd_aio_stat(qp_fd_t* fd);

/**
 * Same with aio_write.
 */
ssize_t
qp_fd_aio_write(qp_fd_t* fd, const void* vptr, size_t nbytes, size_t offset);

/**
 * Same with aio_read.
 */
ssize_t
qp_fd_aio_read(qp_fd_t* fd, void* vptr, size_t nbytes, size_t offset);

#ifdef __cplusplus
}
#endif

#endif 
