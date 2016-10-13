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


#ifndef QP_FILE_H
#define QP_FILE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"


# define  QP_FILE_PATH_LEN_MAX    1024 /* need rewrite */
# define  QP_FILE_DIRECTIO_CACHE  (1024*4) /* 4K */

/*
 * qp_file_t can work just like unix file I/O (beacuse it is unix file I/O), 
 * but provide more prowerful usage, such as cache io (just like FILE struct 
 * in standard I/O) and direct I/O.
*/

# define QP_FILE_NORMAL         0    
# define QP_FILE_DIRECTIO       (1<<0) 
# define QP_FILE_AIO            (1<<1) // not avaliable for now

# define QP_FILE_IS_UNLOCKED    0
# define QP_FILE_IS_LOCKED      1

# ifdef   QP_OS_POSIX
# define  QP_FILE_LK_RD         F_RDLCK
# define  QP_FILE_LK_WR         F_WRLCK
# define  QP_FILE_UNLK          F_UNLCK
# else
# define  QP_FILE_LK_RD         LOCK_SH|LOCK_NB
# define  QP_FILE_LK_WR         LOCK_EX|LOCK_NB
# define  QP_FILE_UNLK          LOCK_UN
# endif 

    
typedef struct qp_file_s*     qp_file_t;


/**
 * Create a file (if file is NULL), and init it.If flag is 0 it works just like 
 * as usual; if mod is QP_FILE_DIRECTIO or QP_FILE_AIO(not support for now), 
 * bufsize mustf be set to spicific cache size.
 */
qp_file_t
qp_file_init(qp_file_t file, qp_int_t flag, size_t bufsize);


qp_int_t
qp_file_destroy(qp_file_t file);


/**
 * Open a file . The oflag and mod are just same with open().
 */
qp_int_t
qp_file_open(qp_file_t file, const qp_char_t* path, qp_int_t oflag, qp_int_t mod);


qp_int_t
qp_file_close(qp_file_t file);


/**
 * Flush data in buffer to disk.
 */
ssize_t
qp_file_flush(qp_file_t file, bool full);


/**
 * Get data from disk to buffer.
 */
ssize_t
qp_file_track(qp_file_t file);


/**
 * If direct IO enabled, qp_file_write_directbuf will get write_direct buffer 
 * pointor to buf and return buf size. You should use it as your data buffer.
 */
size_t
qp_file_get_writebuf(qp_file_t file, qp_uchar_t** buf);


/**
 * If direct IO enabled, qp_file_read_directbuf will get write_direct buffer 
 * pointor to buf and return buf size.
 */
size_t
qp_file_get_readbuf(qp_file_t file, qp_uchar_t** buf);


/**
 * Write to file. If directIO or aio is enabled , you should use 
 * qp_file_get_wrbuf to get the inner write buffer, beacuse vptr will be ignored
 * in those scene and you can pass vptr as NULL.
 */
ssize_t
qp_file_write(qp_file_t file, const void* data, size_t len, size_t file_offset);


/**
 * Read from file.If directIO or aio is enabled , you should use 
 * qp_file_get_rdbuf to get the inner write buffer, beacuse vptr will be ignored
 * in those scene and you can pass vptr as NULL.
 */
ssize_t
qp_file_read(qp_file_t file, void* data, size_t len, size_t file_offset);


/**
 * Direct Write to disk (Use qp_file_t`s buffer, which is got by calling 
 *     qp_file_get_writebuf()).
 */
ssize_t
qp_file_direct_write(qp_file_t file, size_t len);


/**
 * Direct Read to disk (Use qp_file_t`s buffer, which is got by calling 
 *     qp_file_get_readebuf()).
 */
ssize_t
qp_file_direct_read(qp_file_t file, size_t len);


/**
 * Regist file lock on file with type (read or write).Lock the part from 
 * whence + offset with len bytes. 
 */
qp_int_t
qp_file_reglock(qp_file_t file, qp_int_t type, off_t offset, \
    qp_int_t whence, off_t len);


qp_int_t
qp_file_locktest(qp_file_t file, qp_int_t type, off_t offset, \
    qp_int_t whence, off_t len);


qp_int_t
qp_file_rdlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);


qp_int_t
qp_file_wrlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);


qp_int_t
qp_file_unlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);


bool
qp_file_test_rdlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);


bool
qp_file_test_wrlock(qp_file_t file, off_t offset, qp_int_t whence, off_t len);


qp_int_t
qp_file_fd_reglock(qp_int_t fd, qp_int_t type, off_t offset, \
    qp_int_t whence, off_t len);


/* same with qp_file_locktest but use [int] fd */
qp_int_t
qp_file_fd_locktest(qp_int_t fd, qp_int_t type, off_t offset, \
    qp_int_t whence, off_t len);

        
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

#endif /* QP_FILE_H */
