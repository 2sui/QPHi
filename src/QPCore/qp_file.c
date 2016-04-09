
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
qp_file_is_directio(qp_file_t* file) 
{ return file ? file->is_directIO : false; }


qp_file_t*
qp_file_create(qp_file_t *file, qp_int_t mod)
{
    if (NULL == file) {
        file = (qp_file_t*)qp_alloc(sizeof(qp_file_t));
        
        if (NULL == file) {
            QP_LOGOUT_ERROR("[qp_file_t] File create fail.");
            return NULL;
        }

        memset(file, 0, sizeof(qp_file_t));
        QP_FILE_SET_ALLOCED(file);

    } else {
        memset(file, 0, sizeof(qp_file_t));
    }

    /* init qp_fd_t */
    if (NULL == qp_fd_init(&file->file, QP_FD_TYPE_FILE, mod & QP_FILE_AIO)) {

        if (qp_file_is_alloced(file)) {
            qp_free(file);
        }

        QP_LOGOUT_ERROR("[qp_file_t] File fd init fail.");
        return NULL;
    }
    
    file->open_flag = O_RDONLY;
    return file;
}

qp_file_t*
qp_file_init(qp_file_t* file, qp_int_t mod, size_t bufsize)
{
    if (mod && (1 > bufsize)) {
        QP_LOGOUT_ERROR("[qp_file_t] Need set buf size.");
        return NULL;
    }
     
    file = qp_file_create(file, mod);
    
    if (NULL == file) {
        return NULL;
    }

    /* alloc memory for buf rd/wr  */
    if (mod) {
        file->wrbufsize = file->rdbufsize = bufsize;
        
        if (mod & QP_FILE_DIRECTIO) {
            
            if (bufsize < QP_FILE_DIRECTIO_CACHE) {
                bufsize = QP_FILE_DIRECTIO_CACHE;
            }
            
            file->wrbuf = \
                (qp_uchar_t*) qp_alloc_align(QP_PAGE_SIZE, file->wrbufsize);
            file->rdbuf = \
                (qp_uchar_t*) qp_alloc_align(QP_PAGE_SIZE, file->rdbufsize);
            qp_file_set_directIO(file);

        } else {
            file->wrbuf = (qp_uchar_t*) qp_alloc(file->wrbufsize);
            file->rdbuf = (qp_uchar_t*) qp_alloc(file->rdbufsize);
        }

        if (!file->rdbuf || !file->wrbuf) {
            qp_file_destroy(file);
            QP_LOGOUT_ERROR("[qp_file_t] File buf creatr fail.");
            return NULL;
        }
    }
    
    return file;
}

qp_int_t
qp_file_destroy(qp_file_t *file)
{
    if (qp_fd_is_inited(&(file->file))) {
        qp_file_close(file);
        qp_fd_destroy(&(file->file));

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
    if (qp_fd_is_valid(&file->file)) {
        QP_LOGOUT_ERROR("[qp_file_t] File has already used or not inited.");
        return QP_ERROR;
    }
    
    size_t len = strlen(path);
    
    if (len > QP_FILE_PATH_LEN_MAX) {
        QP_LOGOUT_ERROR("[qp_file_t] File name too long.");
        return QP_ERROR;
    }
    
    memcpy(file->name, path, len);
    file->name[len] = '\0';
    
    file->open_flag = oflag;
    file->open_mode = mod;
    
    if (qp_file_is_directio(file)) {
        file->open_flag |= O_DIRECT;
    }
    
    file->file.fd = open(file->name, file->open_flag, file->open_mode);
    
    if (file->file.fd < 0) {
        QP_LOGOUT_ERROR("[qp_file_t] File [%s] open fail.", file->name);
        return QP_ERROR;
    }
    
    
    file->read_offset = file->write_offset = 0;
    file->wrbufoffset = file->wrbufoffsetlast = 0;
    file->rdbufoffset = file->rdbufoffsetlast = 0;
    
    /* get file stat, and set file offset */
    if (QP_ERROR == fstat(file->file.fd, &file->file_stat)) {
        memset(&file->file_stat, 0, sizeof(struct stat));
        
        if (QP_ERROR == (ssize_t)(file->offset = \
            lseek(file->file.fd, 0L, SEEK_END))) 
        {
            file->offset = 0;
            
        }
        
    } else {
        file->offset = file->file_stat.st_size;
    }
    
    if (O_APPEND & file->open_flag) {
        file->read_offset = file->write_offset = file->offset;
        
    } else {
        lseek(file->file.fd, 0L, SEEK_SET);
    }
    
    if (O_TRUNC & file->open_flag) {
        file->offset = 0;
    }
    
    return QP_SUCCESS;
}

qp_int_t
qp_file_close(qp_file_t *file)
{
    if (qp_fd_is_valid(&file->file)) {

        /* rest data in buf to file  */
        if (file->wrbuf && (0 < file->wrbufoffset)) {
            while (qp_file_flush(file) > 0);
        }

        return qp_fd_close(&file->file);
    }

    return QP_ERROR;
}

ssize_t
qp_file_flush(qp_file_t* file)
{   
    if (!file || !file->wrbuf) {
        return QP_ERROR;
    }
    
    if (qp_file_is_directio(file)) {
        qp_fd_write(&file->file, file->wrbuf, file->wrbufoffset);
                
    } else {
        /* coding */
        qp_fd_aio_write(&file->file, file->wrbuf, \
            file->wrbufoffset, file->write_offset);
    }
    
    if (0 < file->file.retsno) {
        file->wrbufoffset = file->wrbufoffset - file->file.retsno;
        
        if (file->wrbufoffset) {
            memcpy(file->wrbuf, file->wrbuf + file->file.retsno, \
                file->wrbufoffset);
        }
        
        file->write_offset += file->file.retsno;
        
        if (file->write_offset > file->offset) {
            file->offset = file->write_offset;
        }
        
        return file->wrbufoffset;
    }
    
    return file->file.retsno;
}

ssize_t 
qp_file_track(qp_file_t* file) 
{
    if (!file || !file->rdbuf) {
        return QP_ERROR;
    }
    
    if (qp_file_is_directio(file)) {
        qp_fd_read(&file->file, file->rdbuf, file->rdbufsize);
        
    } else {
        qp_fd_aio_read(&file->file, file->rdbuf, file->rdbufsize, \
            file->read_offset);
    }
    
    if (0 < file->file.retsno) {
        file->rdbufoffset = file->file.retsno;
        file->rdbufoffsetlast = 0;
        file->read_offset += file->rdbufoffset; 
        return file->rdbufoffset;   
    }
    
    return file->file.retsno;
}

ssize_t
qp_file_write(qp_file_t* file, const void* buf, size_t bufsize)
{
    if (!file || !file->wrbuf) {
        return qp_fd_write(&file->file, buf, bufsize);
        
    } else {
        size_t done = bufsize;
        
        do {
            file->wrbufoffsetlast = file->wrbufoffset;
        
            if (file->wrbufsize > (file->wrbufoffset += bufsize)) {
                memcpy(file->wrbuf + file->wrbufoffsetlast, buf, bufsize);
                return bufsize;
            
            } else {
                /* fill write buf with writing data */
                file->wrbufoffset = file->wrbufsize - file->wrbufoffsetlast;
                memcpy(file->wrbuf + file->wrbufoffsetlast, buf, \
                    file->wrbufoffset);
                buf += file->wrbufoffset;
                bufsize -= file->wrbufoffset;
                file->wrbufoffset = file->wrbufsize;
                
                /* write error */
                if (1 > qp_file_flush(file)) {
                    break;
                }
            }
            
        } while (bufsize); 
        
        if (bufsize == done) {
            return file->file.retsno;
            
        } else {
            return done - bufsize;
        }
        
    }
}

ssize_t
qp_file_read(qp_file_t* file, void* buf, size_t bufsize)
{
    /* buffer read is disabled for now */
    if (!file || !file->rdbuf) {
        return qp_fd_read(&(file->file), buf, bufsize);
        
    } else {
        size_t done = bufsize;
        
        do {
            /* if wanted data size bigger than rest data size in buffer */
            if (bufsize > (file->rdbufoffset - file->rdbufoffsetlast)) {
                memcpy(buf, file->rdbuf + file->rdbufoffsetlast, \
                    file->rdbufoffset - file->rdbufoffsetlast);
                buf += file->rdbufoffset - file->rdbufoffsetlast;
                bufsize -= file->rdbufoffset - file->rdbufoffsetlast;
                
                if (1 > qp_file_track(file)) {
                    break;
                }
                
            } else {
                memcpy(buf, file->rdbuf + file->rdbufoffsetlast, bufsize);
                file->rdbufoffsetlast += bufsize;
                return bufsize;
            }
            
        } while(bufsize);
        
        if (done == bufsize) {
            return file->file.retsno;
        
        } else {
            return done - bufsize;
        }
    }
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
