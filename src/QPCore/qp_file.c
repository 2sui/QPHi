
/**
  * Copyright (C) 2sui.
  */


#include "qp_file.h"


#define  QP_FILE_SET_ALLOCED(file)     (((qp_file_t*)(file))->is_alloced=true)
#define  QP_FILE_SET_DIRECTIO(file)    (((qp_file_t*)(file))->is_directIO=true)

#define  QP_FILE_UNSET_ALLOCED(file)   (((qp_file_t*)(file))->is_alloced=false)
#define  QP_FILE_UNSET_DIRECTIO(file)  (((qp_file_t*)(file))->is_directIO=false)


inline void
qp_file_set_alloced(qp_file_t* file)
{ file ? file->is_alloced = true : 1;}

inline void
qp_file_set_directIO(qp_file_t* file)
{ file ? file->is_directIO = true : 1;}

inline void
qp_file_unset_alloced(qp_file_t* file)
{ file ? file->is_alloced = false : 1;}

inline void
qp_file_unset_directIO(qp_file_t* file)
{ file ? file->is_directIO = false : 1;}


inline bool
qp_file_is_alloced(qp_file_t* file) 
{ return file ? file->is_alloced : false; }

inline bool
qp_file_is_directIO(qp_file_t* file) 
{ return file ? file->is_directIO : false; }

/**
 * Get file stat.
 * 
 * @param file
 * @return Return QP_SUCCESS if success, otherwise return QP_ERROR.
 */
qp_int_t
qp_file_stat(qp_file_t *file) {
    
    if (!file || !qp_fd_is_valid(&file->file)) {
        return QP_ERROR;
    }
    
    file->file.retsno = fstat(file->file.fd, &file->stat);
    file->file.errono = errno;
    return file->file.retsno;
}

qp_file_t*
qp_file_create(qp_file_t *file, qp_int_t mod)
{
    if (NULL == file) {
        file = (qp_file_t*)qp_alloc(sizeof(qp_file_t));
        
        if (NULL == file) {
            return NULL;
        }

        memset(file, 0, sizeof(qp_file_t));
        QP_FILE_SET_ALLOCED(file);

    } else {
        memset(file, 0, sizeof(qp_file_t));
    }

    /* init qp_fd_t */
    if (NULL == qp_fd_init(&file->file, QP_FD_TYPE_FILE, mod & 0/*QP_FILE_AIO*/)){

        if (qp_file_is_alloced(file)) {
            qp_free(file);
        }

        return NULL;
    }
    
    file->open_flag = O_RDONLY;
    return file;
}

qp_file_t*
qp_file_init(qp_file_t* file, qp_int_t mod, size_t bufsize)
{
    file = qp_file_create(file, mod);
    
    if (NULL == file) {
        return NULL;
    }
    
    /*
     * DOES NOT SUPPORT AIO!
     */
    mod &= ~QP_FILE_AIO;
    
    if ((QP_FILE_DIRECTIO & mod) && (1 > bufsize)) {
        file->wrbuf_size = file->rdbuf_size = QP_FILE_DIRECTIO_CACHE;
    }
    
    switch (mod) {
        
    case QP_FILE_DIRECTIO: {
        file->wrbuf = (qp_uchar_t*) qp_alloc_align(QP_PAGE_SIZE, file->wrbuf_size);
        file->rdbuf = (qp_uchar_t*) qp_alloc_align(QP_PAGE_SIZE, file->rdbuf_size);
        qp_file_set_directIO(file);
            
    } break;
    
    default: break;
    } // switch
    
    if (QP_FILE_DIRECTIO & mod) {
        
        if (!file->rdbuf || !file->wrbuf) {
            qp_file_destroy(file);
            return NULL;
        }
    }
    
    return file;
}

qp_int_t
qp_file_destroy(qp_file_t *file)
{
    if (file && qp_fd_is_inited(&file->file)) {
        qp_file_close(file);
        qp_fd_destroy(&file->file);

        if (NULL != file->wrbuf) {
            qp_free(file->wrbuf);
            file->wrbuf = NULL;
        }
        
        if (NULL != file->rdbuf) {
            qp_free(file->rdbuf);
            file->rdbuf = NULL;
        }
        
        qp_file_unset_directIO(file);

        if (qp_file_is_alloced(file)) {
            qp_free(file);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_file_open(qp_file_t* file, qp_char_t* path, qp_int_t oflag, qp_int_t mod)
{
    /* file has been opened */
    if (!file || qp_fd_is_valid(&file->file) 
        || (strlen(path) > QP_FILE_PATH_LEN_MAX)) 
    {
        return QP_ERROR;
    }
    
    memcpy(file->name, path, strlen(path));
    file->name[strlen(path)] = '\0';
    
    file->open_flag = oflag;
    file->open_mode = mod;
    
    if (qp_file_is_directIO(file)) {
        file->open_flag |= O_DIRECT;
    }
    
    file->file.fd = open(file->name, file->open_flag, file->open_mode);
    
    if (file->file.fd < 0) {
        return QP_ERROR;
    }
    
    if (QP_ERROR == qp_file_stat(file)) {
        memset(&file->stat, 0, sizeof(struct stat));
    }
    
    if (O_APPEND & file->open_flag) {
        file->cur_file_offset = file->stat.st_size;
        
    } else {
        file->cur_file_offset = 0;
    }
    
    return QP_SUCCESS;
}

qp_int_t
qp_file_close(qp_file_t *file)
{
    if (!file) {
        return QP_ERROR;    
    }
    
    if (qp_fd_is_valid(&file->file)) {

        /* rest data in buf to file  */
        if (file->wrbuf && (0 < file->wrbuf_offset)) {
            while (qp_file_flush(file, false) > 0);
        }

        return qp_fd_close(&file->file);
    }

    return QP_ERROR;
}

ssize_t
qp_file_flush(qp_file_t* file, bool full)
{   
    if (!file || !file->wrbuf) {
        return QP_ERROR;
    }
    
    if (qp_file_is_directIO(file)) {
        qp_fd_write(&file->file, file->wrbuf, full ? \
            file->wrbuf_size : file->wrbuf_offset);
    } 
    
    if (0 < file->file.retsno) {
        file->wrbuf_offset = file->wrbuf_offset - file->file.retsno;
        
        if (file->wrbuf_offset) {
            memcpy(file->wrbuf, file->wrbuf + file->file.retsno, \
                file->wrbuf_offset);
        }
        
        qp_file_stat(file);
        
        return file->wrbuf_size - file->wrbuf_offset;
    }
    
    return file->file.retsno;
}

ssize_t 
qp_file_track(qp_file_t* file) 
{
    if (!file || !file->rdbuf) {
        return QP_ERROR;
    }
    
    if (qp_file_is_directIO(file)) {
        qp_fd_read(&file->file, file->rdbuf, file->rdbuf_size);
    } 
    
    if (0 < file->file.retsno) {
        file->rdbuf_offset = file->file.retsno;
        file->rdbuf_offsetlast = 0;
        return file->rdbuf_offset;   
    }
    
    return file->file.retsno;
}

size_t
qp_file_get_writebuf(qp_file_t* file, qp_uchar_t** buf) 
{
    if (!file || !qp_fd_is_inited(&file->file) || !qp_file_is_directIO(file)) {
        return 0;
    }
    
    *buf = file->wrbuf;
    return  file->wrbuf_size;
}

size_t
qp_file_get_readbuf(qp_file_t* file, qp_uchar_t** buf) 
{
    if (!file || !qp_fd_is_inited(&file->file) || !qp_file_is_directIO(file)) {
        return 0;
    }
    
    *buf = file->rdbuf;
    return  file->rdbuf_size;
}

ssize_t
qp_file_write(qp_file_t* file, const void* data, size_t len, size_t file_offset)
{
    if (!file || !qp_fd_is_inited(&file->file)) {
        return QP_ERROR;
    }
    
    if (!qp_file_is_directIO(file)) {
        return qp_fd_is_aio(&file->file) ? \
            qp_fd_write(&file->file, data, len) :\
            qp_fd_aio_write(&file->file, data, len, file_offset);
        
    } else {
        size_t done = len;
        
        do {
            file->wrbuf_offsetlast = file->wrbuf_offset;
        
            if (file->wrbuf_size > (file->wrbuf_offset += len)) {
                memcpy(file->wrbuf + file->wrbuf_offsetlast, data, len);
                return len;
            
            } else {
                /* fill write buf with writing data */
                file->wrbuf_offset = file->wrbuf_size - file->wrbuf_offsetlast;
                memcpy(file->wrbuf + file->wrbuf_offsetlast, data, \
                    file->wrbuf_offset);
                data += file->wrbuf_offset;
                len -= file->wrbuf_offset;
                
                /* write error */
                if (1 > qp_file_flush(file, true)) {
                    break;
                }
            }
            
        } while (len); 
        
        if (len) {
            return file->file.retsno;
        } 
        
        return done;
    }
}

ssize_t
qp_file_read(qp_file_t* file, void* data, size_t len, size_t file_offset)
{
    if (!file && !qp_fd_is_inited(&file->file)) {
        return QP_ERROR;
    }
    
    /* buffer read is disabled for now */
    if (!qp_file_is_directIO(file)) {
        return qp_fd_is_aio(&file->file) ? \
            qp_fd_read(&file->file, data, len) :\
            qp_fd_aio_read(&file->file, data, len, file_offset);
        
    } else {
        size_t done = len;
        size_t rest = 0;
        
        do {
            rest = file->rdbuf_offset - file->rdbuf_offsetlast;
            
            /* if wanted data size bigger than rest data size in buffer */
            if (len > rest) {
                memcpy(data, file->rdbuf + file->rdbuf_offsetlast, rest);
                data += rest;
                len -= rest;
                
                if (1 > qp_file_track(file)) {
                    break;
                }
                
            } else {
                memcpy(data, file->rdbuf + file->rdbuf_offsetlast, len);
                file->rdbuf_offsetlast += len;
                return len;
            }
            
        } while(len);
        
        if (len) {
            return file->file.retsno;
        
        } 
        
        return done;
    }
}

ssize_t
qp_file_direct_write(qp_file_t* file, size_t len)
{
    if (!file && !qp_fd_is_inited(&file->file)) {
        return QP_ERROR;
    }
    
    return qp_fd_write(&file->file, file->wrbuf, len);
}

qp_int_t
qp_file_reglock(qp_file_t *file, qp_int_t type, 
    off_t offset, qp_int_t whence, off_t len)
{
    if (!file) {
        return QP_ERROR;
    }
#ifdef QP_OS_POSIX

#ifdef __USE_LARGEFILE64
    struct flock64  lock;
#else
    sturct flock      flock;
#endif
    lock.l_type = (qp_short_t)type;
    lock.l_start = offset;
    lock.l_whence = (qp_short_t)whence;
    lock.l_len = len;

#ifdef __USE_LARGEFILE64
    return fcntl(file->file.fd, F_SETLK64, &lock);
#else
    return fcntl(file->file.fd, F_SETLK, &lock);
#endif

#else
    return flock(file->file.fd, type);
#endif
}

qp_int_t
qp_file_locktest(qp_file_t *file, qp_int_t type, 
    off_t offset, qp_int_t whence, off_t len)
{
    if (!file) {
        return QP_ERROR;
    }
#ifdef QP_OS_POSIX

#ifdef __USE_LARGEFILE64
    struct flock64  lock;
#else
    sturct flock      flock;
#endif
    lock.l_type = (qp_short_t)type;
    lock.l_start = offset;
    lock.l_whence = (qp_short_t)whence;
    lock.l_len = len;

#ifdef __USE_LARGEFILE64
    if (0 > fcntl(file->file.fd, F_GETLK64, &lock))
#else
    if (0 > fcntl(file->file.fd, F_GETLK, &lock))
#endif
    {
        return QP_ERROR;
    }

    if (F_UNLCK == lock.l_type) {
        return QP_FILE_IS_UNLOCKED;
    }

    return QP_FILE_IS_LOCKED;

#else
    if (0 == flock(file->file.fd, type)) {
        flock(file->file.fd, QP_FILE_UNLK);
        return QP_FILE_IS_UNLOCKED;

    } else {

        if (EWOULDBLOCK == errno) {
            return QP_FILE_IS_LOCKED;

        } else {
            return QP_ERROR;
        }
    }
#endif
}

/* read lock */
qp_int_t
qp_file_rdlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len)
{ return qp_file_reglock(file, QP_FILE_LK_RD, offset, whence, len); }

/* write lock */
qp_int_t
qp_file_wrlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len)
{ return qp_file_reglock(file, QP_FILE_LK_WR, offset, whence, len); }

/* unlock */
qp_int_t
qp_file_unlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len)
{ return qp_file_reglock(file, QP_FILE_UNLK, offset, whence, len); }

/* test read lock */
bool
qp_file_test_rdlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len)
{ return (QP_FILE_IS_UNLOCKED == \
    qp_file_locktest(file, QP_FILE_LK_RD, offset, whence, len)); }

/* test write lock */
bool
qp_file_test_wrlock(qp_file_t* file, off_t offset, qp_int_t whence, off_t len)
{ return (QP_FILE_IS_UNLOCKED == \
    qp_file_locktest(file, QP_FILE_LK_WR, offset, whence, len)); }


qp_int_t
qp_file_fd_reglock(qp_int_t fd, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len)
{
    qp_file_t  file;
    memset(&file, 0, sizeof(file));
    
    file.file.fd = fd;
    return qp_file_reglock(&file, type, offset, whence, len);
}

qp_int_t
qp_file_fd_locktest(qp_int_t fd, qp_int_t type, off_t offset, qp_int_t whence, \
    off_t len)
{
    qp_file_t  file;
    memset(&file, 0, sizeof(file));
    
    file.file.fd = fd;
    return qp_file_locktest(&file, type, offset, whence, len);
}

/* read lock */
qp_int_t
qp_file_fd_rdlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len)
{ return qp_file_fd_reglock(fd, QP_FILE_LK_RD, offset, whence, len); }

/* write lock */
qp_int_t
qp_file_fd_wrlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len)
{ return qp_file_fd_reglock(fd, QP_FILE_LK_WR, offset, whence, len); }

/* unlock */
qp_int_t
qp_file_fd_unlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len)
{ return qp_file_fd_reglock(fd, QP_FILE_UNLK, offset, whence, len); }

/* test read lock */
bool
qp_file_fd_test_rdlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len)
{ return (QP_FILE_IS_UNLOCKED == \
    qp_file_fd_locktest(fd, QP_FILE_LK_RD, offset, whence, len)); }

/* test write lock */
bool
qp_file_fd_test_wrlock(qp_int_t fd, off_t offset, qp_int_t whence, off_t len)
{ return (QP_FILE_IS_UNLOCKED == \
    qp_file_fd_locktest(fd, QP_FILE_LK_WR, offset, whence, len)); }
