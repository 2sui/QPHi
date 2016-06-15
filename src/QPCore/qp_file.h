

/**
  * Copyright (C) 2sui.
  *
  * File I/O operations : basic I/O, direct I/O (main use), AIO.
  * 
  * The buffer read function is disabled for now. 
  */


#ifndef QP_FILE_H
#define QP_FILE_H


#include "qp_o_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  QP_FILE_PATH_LEN_MAX    1024 /* need rewrite */
#define  QP_FILE_DIRECTIO_CACHE  (1024*4) /* 4K */

/*
 * qp_file_t can work just like unix file I/O (beacuse it is unix file I/O), 
 * but provide more prowerful usage, such as cache io (just like FILE struct 
 * in standard I/O) and direct I/O.
*/

#define QP_FILE_NORMAL         0    
#define QP_FILE_DIRECTIO       (1<<0) 
#define QP_FILE_AIO            (1<<1) // not avaliable for now

#define QP_FILE_IS_UNLOCKED    0
#define QP_FILE_IS_LOCKED      1

#ifdef   QP_OS_POSIX
#define  QP_FILE_LK_RD         F_RDLCK
#define  QP_FILE_LK_WR         F_WRLCK
#define  QP_FILE_UNLK          F_UNLCK
#else
#define  QP_FILE_LK_RD         LOCK_SH|LOCK_NB
#define  QP_FILE_LK_WR         LOCK_EX|LOCK_NB
#define  QP_FILE_UNLK          LOCK_UN
#endif 

typedef struct qp_file_s*     qp_file_t;


bool
qp_file_is_alloced(qp_file_t file);

bool
qp_file_is_directIO(qp_file_t file);

/**
 * Create a file (if file is NULL), and init it.If mod is 0 it works just like 
 * as usual; if mod is QP_FILE_DIRECTIO or QP_FILE_AIO(not support for now), 
 * bufsize mustf be set to spicific cache size.
 * 
 * @param file: Your qp_file_t struct.
 * @param mod: QP_FILE_NORMAL,QP_FILE_DIRECTIO...
 * @param bufsize: Read/write buf size if you use QP_FILE_DIRECTIO.
 * @return Return qp_file_t pointer if success, otherwise return NULL.
 */
qp_file_t
qp_file_init(qp_file_t file, qp_int_t mod, size_t bufsize);

/**
 * Destory an inited file(which is called by qp_file_init or qp_file_create).
 * 
 * @param file: Valid qp_file_t.
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
qp_int_t
qp_file_destroy(qp_file_t file);

/**
 * Open a file . The oflag and mod are just same with open().
 * 
 * @param file: Valid qp_file_t.
 * @param path: File path.
 * @param oflag: Open flag.(O_WRONLY,O_RDONLY,O_RDWR,O_CREAT...)
 * @param mod: S_IRUSR,S_IWUSR,S_IXUSR...
 * @return Return QP_SUCCESS if success, otherwise return QP_ERROR.
 */
qp_int_t
qp_file_open(qp_file_t file, const qp_char_t* path, qp_int_t oflag, qp_int_t mod);

/**
 * Close a opened file.
 * 
 * @param file: Valid qp_file_t.
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
qp_int_t
qp_file_close(qp_file_t file);

/**
 * Flush data in buffer to disk.
 * 
 * @param file: Valid qp_file_t.
 * @param full: Flush all data in buffer to disk.
 * @return Return number of rest data in buffer, and return QP_ERROR if some 
 *     error happen.
 */
ssize_t
qp_file_flush(qp_file_t file, bool full);

/**
 * Get data from disk to buffer.
 * 
 * @param file: Valid qp_file_t.
 * @return Return number of data read to buffer, and return QP_ERROR if some 
 *     error happen.
 */
ssize_t
qp_file_track(qp_file_t file);

/**
 * If direct IO enabled, qp_file_write_directbuf will get write_direct buffer 
 * pointor to buf and return buf size. You should use it as your data buffer.
 * 
 * @param file: Valid qp_file_t.
 * @param buf: Pointer that will be set with write buffer address.
 * @return Write buffer size.
 */
size_t
qp_file_get_writebuf(qp_file_t file, qp_uchar_t** buf);

/**
 * If direct IO enabled, qp_file_read_directbuf will get write_direct buffer 
 * pointor to buf and return buf size.
 * 
 * @param file: Valid qp_file_t.
 * @param buf: Pointer that will be set with read buffer address.
 * @return  Read buffer size.
 */
size_t
qp_file_get_readbuf(qp_file_t file, qp_uchar_t** buf);

/**
 * Write to file. If directIO or aio is enabled , you should use 
 * qp_file_get_wrbuf to get the inner write buffer, beacuse vptr will be ignored
 * in those scene and you can pass vptr as NULL.
 * 
 * @param file: Valid qp_file_t.
 * @param data: Write buffer.
 * @param len: Write size.
 * @param file_offset: File offset (for AIO)
 * @return Reutrn write size or QP_ERROR if some error happen.
 */
ssize_t
qp_file_write(qp_file_t file, const void* data, size_t len, size_t file_offset);

/**
 * Read from file.If directIO or aio is enabled , you should use 
 * qp_file_get_rdbuf to get the inner write buffer, beacuse vptr will be ignored
 * in those scene and you can pass vptr as NULL.
 * 
 * @param file: Valid qp_file_t.
 * @param data: Read buffer.
 * @param len: Max read len.
 * @param file_offset: File offet (only for AIO)
 * @return Return QP_SUCCESS if success otherwise return QP_ERROR.
 */
ssize_t
qp_file_read(qp_file_t file, void* data, size_t len, size_t file_offset);

/**
 * Direct Write to disk (Use qp_file_t`s buffer, which is got by calling 
 *     qp_file_get_writebuf()).
 * 
 * @param file: Valid qp_file_t.
 * @param len: Data length to write.
 * @return Return size of written buffer, and QP_ERROR if some error happen.
 */
ssize_t
qp_file_direct_write(qp_file_t file, size_t len);

/**
 * Direct Read to disk (Use qp_file_t`s buffer, which is got by calling 
 *     qp_file_get_readebuf()).
 * 
 * @param file: Valid qp_file_t.
 * @param len: Data length to read.
 * @return Return size of read buffer, and QP_ERROR if some error happen.
 */
ssize_t
qp_file_direct_read(qp_file_t file, size_t len);

/**
 * Regist file lock on file with type (read or write).Lock the part from 
 * whence + offset with len bytes. 
 * 
 * @return If success return QP_SUCCESS, otherwise return QP_ERROR.
 */
qp_int_t
qp_file_reglock(qp_file_t file, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len);

/**
 * Test file lock. 
 * 
 * @return If some error happend return QP_ERROR, and if file is locked, return 
 *     QP_FILE_IS_LOCKED, otherwise return QP_FILE_IS_UNLOCKED.
 */
qp_int_t
qp_file_locktest(qp_file_t file, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len);

/**
 * read lock 
 * 
 * @return Return QP_SUCCESS if success , otherwise return QP_ERROR.
 */
qp_int_t
qp_file_rdlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);

/**
 *  write lock 
 * 
 * @return Return QP_SUCCESS if success , otherwise return QP_ERROR.
 */
qp_int_t
qp_file_wrlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);

/** 
 * unlock 
 * 
 * @return Return QP_SUCCESS if success , otherwise return QP_ERROR.
 */
qp_int_t
qp_file_unlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);

/**
 *  test read lock 
 * @return If file is locked return false, otherwise return true.
 */
bool
qp_file_test_rdlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);

/** 
 * test write lock
 * 
 * @return If file is locked return false, otherwise return true. 
 */
bool
qp_file_test_wrlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);

qp_int_t
qp_file_fd_reglock(qp_int_t fd, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len);

/* same with qp_file_locktest but use [int] fd */
qp_int_t
qp_file_fd_locktest(qp_int_t fd, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len);
        
/* read lock */
qp_int_t
qp_file_fd_rdlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len);

/* write lock */
qp_int_t
qp_file_fd_wrlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len);

/* unlock */
qp_int_t
qp_file_fd_unlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len);

/* test read lock */
bool
qp_file_fd_test_rdlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len);

/* test write lock */
bool
qp_file_fd_test_wrlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len);

#ifdef __cplusplus
}
#endif

#endif 
