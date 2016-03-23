

/**
  * Copyright (C) 2sui.
  *
  * File I/O operations : basic I/O, direct I/O (main use), AIO.
  * 
  * The buffer read function is disabled for now. 
  */


#ifndef QP_FILE_H
#define QP_FILE_H


#ifdef __cplusplus
extern "C" {
#endif


#include "qp_o_io.h"
    
    
#define  QP_FILE_PATH_LEN_MAX    1024 /* need rewrite */
#define  QP_FILE_DIRECTIO_CACHE  (1024*16) /* 16K */

/*
 * qp_file_t can work just like unix file I/O (beacuse it is unix file I/O), 
 * but provide more prowerful usage, such as cache io (just like FILE struct 
 * in standard I/O) and direct I/O.
*/
    
#define QP_FILE_DIRECTIO       (1<<0)
#define QP_FILE_AIO            (1<<1)

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


struct qp_file_s {
    qp_fd_t                  file;         /* file path */
    qp_int_t                 mod;
    qp_int_t                 open_flag;    /* O_RDONLY O_WRONLY O_RDWR O_EXEC O_SEARCH */
    qp_int_t                 open_opt;     /* O_CREAT O_TRUC ... */
    qp_int_t                 open_mode;    /* IR_USR IW_GRP IX_OTH ... */
    qp_uchar_t*              wrbuf;        /* file write/read buf */
    qp_uchar_t*              rdbuf;
    size_t                   wrbufsize;    /* file buf size */
    size_t                   rdbufsize;
    size_t                   wrbufoffset;  /* file buf offset */
    size_t                   wrbufoffsetlast;  /* file buf offset */
    size_t                   rdbufoffset;
    size_t                   rdbufoffsetlast;
    size_t                   offset;       /* file offset */
    size_t                   write_offset;
    size_t                   read_offset;
    struct stat              file_stat;    /* file stat */
    qp_char_t                name[QP_FILE_PATH_LEN_MAX + 1];
    bool                     is_directIO;  /* use direct io */
    bool                     is_alloced;   /* is this struct allocated */
};

typedef struct qp_file_s     qp_file_t;


inline bool
qp_file_is_alloced(qp_file_t* file);

inline bool
qp_file_is_directio(qp_file_t* file);

/*
 * Create a file and init it (if file is NULL, otherwise just init it).If 
 * bufsize is not 0, it will use writecache(which size is bufsize), and
 * if directIO is TRUE, bufsize must be set( It will use direct IO).
 * Return qp_file_t pointer if success, otherwise return NULL.
*/
qp_file_t*
qp_file_init(qp_file_t* file, qp_int_t mod, size_t bufsize);

/*
 * Destory an inited file(which is called by qp_file_init or qp_file_create).
*/
qp_int_t
qp_file_destroy(qp_file_t* file);

/*
 * Open a file .
 * If success return QP_SUCCESS, otherwise return QP_ERROR.
 * The oflag and mod are just same with open().
*/
qp_int_t
qp_file_open(qp_file_t* file, qp_char_t* path, qp_int_t oflag, qp_int_t mod);

/*
 * Close a opened file.
*/
qp_int_t
qp_file_close(qp_file_t* file);

/**
 * Flush data in buffer to disk.
 * Return number of rest data in buffer, and return QP_ERROR if some error happen.
 */
ssize_t
qp_file_flush(qp_file_t* file);

/**
 * Get data from disk to buffer.
 */
ssize_t
qp_file_track(qp_file_t* file);

/*
 * Write  to file.If directIO or aio is enabled , you should use 
 * qp_file_get_wrbuf to get the inner write buffer, beacuse vptr will be ignored
 * in those scene and you can pass vptr as NULL.
*/
ssize_t
qp_file_write(qp_file_t* file, const void* vptr, \
    size_t bufsize);

/*
 * Read from file.If directIO or aio is enabled , you should use 
 * qp_file_get_rdbuf to get the inner write buffer, beacuse vptr will be ignored
 * in those scene and you can pass vptr as NULL.
 * 
 *  ** for now,  buffer cache read is not enabled, so it is read directly from file **
*/
ssize_t
qp_file_read(qp_file_t* file, void* vptr, size_t bufsize);

/*
 * Regist file lock on file with type (read or write).Lock the part from 
 * whence + offset with len bytes. If success return QP_SUCCESS, 
 * otherwise return QP_ERROR.
*/
qp_int_t
qp_file_reglock(qp_file_t *file, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len);

/*
 * Test file lock. If some error happend return QP_ERROR, and if file is locked, 
 * return QP_FILE_IS_LOCKED, otherwise return QP_FILE_IS_UNLOCKED.
*/
qp_int_t
qp_file_locktest(qp_file_t* file, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len);

/* read lock */
qp_int_t
qp_file_rdlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len);

/* write lock */
qp_int_t
qp_file_wrlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len);

/* unlock */
qp_int_t
qp_file_unlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len);

/* test read lock */
bool
qp_file_test_rdlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len);

/* test write lock */
bool
qp_file_test_wrlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len);

/* same with qp_file_reglock but use [int] fd */
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
