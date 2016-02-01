/**
 * lock.c
 * 
 * Test for qpCore module.
 */


#include <qphi.h>

static qp_shm_t       *shm = NULL;
static qp_process_t   *child = NULL;

typedef union {
    qp_lock_t    lock;
    qp_rwlock_t  rwlock;
    qp_cond_t    cond;
    qp_sem_t     sem;
} box;

#define INFO(info...)  fprintf(stderr, "\n"info)

int 
test_shm()
{
    if (shm) {
        qp_shm_destroy(shm);
        shm = NULL;
        return QP_SUCCESS;
    }
    
    if (!(shm = qp_shm_init(shm, sizeof(unsigned long) + sizeof(box), NULL))) {
        INFO("Shm init fail.");
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}

int
test_process()
{
    if (child) {
        qp_process_destroy(child);
        child = NULL;
        return QP_SUCCESS;
    }
    
    if (!(child = qp_process_init(child))) {
        INFO("Process init fail.");
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}

void
test_lock()
{   
    INFO(">>>>>>>>>>> Test for lock.");
    
    unsigned long *count = (unsigned long*) qp_shm_start(shm);
    qp_lock_t* lock = (qp_lock_t*) (qp_shm_start(shm) + sizeof(unsigned long));
    
    if (!lock) {
        INFO("Lock ptr get fail.");
        return;
    }
    
    if (!qp_lock_init(lock, true, false)) {
        INFO("Lock init fail.");
        return;
    }
    
    *count = 0;
    
    if (QP_ERROR == qp_process_start(child)) {
        qp_lock_destroy(lock);
        INFO("Start process fail.");
        return;
    }
    
    if (0 == child->pid) {
        int i = 0;
        
        for (; i < 1000; i++) {
            qp_lock_lock(lock);
            INFO("Child do: %lu.", ++(*count));
            qp_lock_unlock(lock);
        }
        exit(0);
    }
    
    int i = 0;
    usleep(10000);
    for (; i < 1000; i++) {
        qp_lock_lock(lock);
        INFO("Parent do: %lu.", ++(*count));
        qp_lock_unlock(lock);
    }
    
    qp_process_stop(child, false);
    
    if (QP_ERROR == qp_lock_destroy(lock)) {
        qp_lock_unlock(lock);
        qp_lock_destroy(lock);
    }
    
    INFO(">>>>>>>>>>> Test for lock done.");
}

void
test_spin()
{
    INFO(">>>>>>>>>>> Test for spin.");
    
    unsigned long *count = (unsigned long*) qp_shm_start(shm);
    qp_lock_t* lock = (qp_lock_t*) (qp_shm_start(shm) + sizeof(unsigned long));
    
    if (!lock) {
        INFO("Lock ptr get fail.");
        return;
    }
    
    if (!qp_lock_init(lock, true, true)) {
        INFO("Lock init fail.");
        return;
    }
    
    *count = 0;
    
    if (QP_ERROR == qp_process_start(child)) {
        qp_lock_destroy(lock);
        INFO("Start process fail.");
        return;
    }
    
    if (0 == child->pid) {
        int i = 0;
        
        for (; i < 1000; i++) {
            qp_lock_lock(lock);
            INFO("Child do: %lu.", ++(*count));
            qp_lock_unlock(lock);
        }
        exit(0);
    }
    
    int i = 0;
    usleep(10000);
    for (; i < 1000; i++) {
        qp_lock_lock(lock);
        INFO("Parent do: %lu.", ++(*count));
        qp_lock_unlock(lock);
    }
    
    qp_process_stop(child, false);
    
    if (QP_ERROR == qp_lock_destroy(lock)) {
        qp_lock_unlock(lock);
        qp_lock_destroy(lock);
    }
    
    INFO(">>>>>>>>>>> Test for spin done.");
}

void
test_rwlock()
{
    INFO(">>>>>>>>>>> Test for rwlock.");
    
    unsigned long *count = (unsigned long*) qp_shm_start(shm);
    qp_rwlock_t* lock = (qp_rwlock_t*)(qp_shm_start(shm) + sizeof(unsigned long));
    
    if (!lock) {
        INFO("RWLock ptr get fail.");
        return;
    }
    
    if (!qp_rwlock_init(lock, true)) {
        INFO("RWLock init fail.");
        return;
    }
    
    *count = 0;
    
    if (QP_ERROR == qp_process_start(child)) {
        qp_rwlock_destroy(lock);
        INFO("Start process fail.");
        return;
    }
    
    if (0 == child->pid) {
        int i = 0;
        
        for (; i < 10; i++) {
            qp_rwlock_wrlock(lock);
            INFO("Child wr: %lu.", ++(*count));
            sleep(3);
            qp_rwlock_unlock(lock);
            qp_rwlock_rdlock(lock);
            sleep(4);
            qp_rwlock_unlock(lock);
        }
        exit(0);
    }
    
    for (; ; ) {
        qp_rwlock_rdlock(lock);
        INFO("Parent rd: %lu.", (*count));
        qp_rwlock_unlock(lock);
        
        if (*count == 10) break;
    }
    
    qp_process_stop(child, false);
    
    if (QP_ERROR == qp_rwlock_destroy(lock)) {
        qp_rwlock_unlock(lock);
        qp_rwlock_destroy(lock);
    }
    
    INFO(">>>>>>>>>>> Test for rwlock done.");
}

void
test_cond()
{
    INFO(">>>>>>>>>>> Test for cond.");
    
    unsigned long *count = (unsigned long*) qp_shm_start(shm);
    qp_cond_t* cond = (qp_cond_t*)(qp_shm_start(shm) + sizeof(unsigned long));
    
    if (!cond) {
        INFO("Cond ptr get fail.");
        return;
    }
    
    if (!qp_cond_init(cond, true)) {
        INFO("Cond init fail.");
        return;
    }
    
    *count = 0;
    
    if (QP_ERROR == qp_process_start(child)) {
        qp_cond_destroy(cond);
        INFO("Start process fail.");
        return;
    }
    
    if (0 == child->pid) {
        int i = 0;
        
        for (; i < 5; i++) { 
            sleep(2);
            INFO("Signal: %lu.", ++(*count));
            qp_cond_signal(cond, NULL, NULL);
            sleep(3);
        }
        exit(0);
    }
    unsigned long cur = 0;
    for (; ; ) {
        qp_cond_wait(cond, NULL, NULL);
        cur = (*count);
        INFO("Wait: %lu.", cur);
        
        if (cur == 5) break;
    }
    
    qp_process_stop(child, false);
    
    if (QP_ERROR == qp_cond_destroy(cond)) {
        qp_cond_signal(cond, NULL, NULL);
        qp_cond_destroy(cond);
    }
    
    INFO(">>>>>>>>>>> Test for cond done.");
}

void
test_sem()
{
    INFO(">>>>>>>>>>> Test for sem.");
    
    unsigned long *count = (unsigned long*) qp_shm_start(shm);
    qp_sem_t* sem = (qp_sem_t*)(qp_shm_start(shm) + sizeof(unsigned long));
    
    if (!sem) {
        INFO("Cond ptr get fail.");
        return;
    }
    
    if (!qp_sem_init(sem, true)) {
        INFO("Sem init fail.");
        return;
    }
    
    *count = 0;
    
    if (QP_ERROR == qp_process_start(child)) {
        qp_sem_destroy(sem);
        INFO("Start process fail.");
        return;
    }
    
    if (0 == child->pid) {
        int i = 0;
        
        for (; i < 5; i++) {
            sleep(2);
            INFO("Post: %lu", ++(*count));
            qp_sem_post(sem);
            sleep(3);
        }
        
        exit(0);
    }
    
    for (; ; ) {
        qp_sem_wait(sem);
        INFO("Wait: %lu", (*count));
        
        if (*count == 5) break;
    }
    
    qp_process_stop(child, false);
    
    qp_sem_destroy(sem);
    
    INFO(">>>>>>>>>>> Test for sem done.");
}


int 
main(int argc, char** argv)
{
    if (QP_ERROR == test_shm()) {
        return QP_ERROR;
    }
    
    if (QP_ERROR == test_process()) {
        test_shm();
        return QP_ERROR;
    }
    
//    test_lock();
    INFO();
//    test_spin();
    INFO();
//    test_rwlock();
    INFO();
//    test_cond();
    INFO();
    test_sem();
    
    test_process();
    test_shm();
    
    return QP_SUCCESS;
}

