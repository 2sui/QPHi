
#include <qp_socket.h>
#include <qp_atomic.h>


#define  WORKER_PROCESS_NUM    4
#define  TESTCASE_THREAD_NUM    6
#define  LISTENER_ADDR                "0.0.0.0"
#define  LISTENER_PORT                 8888
#define  DATA_BUF_SIZE                  1024


qp_int_t master_process_cycle();
qp_int_t worker_process_cycle(int index);
qp_int_t test_case();


qp_socket_t     listener;
pid_t               pid[WORKER_PROCESS_NUM + 1] = { 0 };
int                  sktpair[WORKER_PROCESS_NUM][2] = { 0 };
bool                procstat = true;


int
main(int argc, char *argv[])
{
    /* start test client */
    if (argc > 1) {
        if (0 == strcmp("testcase", argv[1])) {
            return test_case();
        }
    }

    /* init socket */
    if (NULL == qp_socket_init(&listener, AF_INET, NULL, LISTENER_ADDR, LISTENER_PORT, true)) {
        fprintf(stderr, "\nSocket init fail.");
        return -1;
    }

    qp_socket_set_attr(&listener, QP_SO_REUSEADDR, NULL, 0);

    qp_socket_init_attr(&listener);

    /* listen and bind */
    if (QP_ERROR == qp_socket_listen(&listener)) {
        qp_socket_destory(&listener);
        fprintf(stderr, "\nSocket listen fail.");
        return -1;
    }

    pid[0] = getpid();

    /* start accept process (the pipe fd is useless, it`s just for fun...) */
    for (int i = 1; i < WORKER_PROCESS_NUM + 1; i++) {
        pipe(sktpair[i - 1]);
        pid[i] = fork();
        if (pid[i] < 0) {

            for (int j = 1; j <= i; j++) {
                    close(sktpair[j - 1][1]);

                    if (i == j) {
                        close(sktpair[j - 1][0]);
                    }
            }

            return -1;

        } else {
            if (0 == pid[i]) {
                return worker_process_cycle(i);
            }
        }

        /* close all reading end */
        close(sktpair[i - 1][0]);
    }

    return master_process_cycle();
}

void
sig_handler(qp_int_t signo)
{
    procstat = false;
}

int
master_process_cycle()
{
    sigset_t     sigs;
    qp_int_t             signo = 0;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGHUP);
    sigprocmask(SIG_BLOCK, &sigs, NULL);

    while (1) {
        sigwait(&sigs, &signo);

        switch (signo) {

        case SIGINT:
        case SIGHUP: {

            /* It`s simple and crude to kill all accept processes */
            for (int i = 1; i < WORKER_PROCESS_NUM + 1; i++) {
                kill(pid[i], SIGKILL);
                waitpid(pid[i], NULL, 0);
            }

            return 0;
        }
            break;
        default:
            break;
        }
    }
}

int
worker_process_cycle(int index)
{
    qp_socket_t  client;
//    qp_socket_attr_t  skt_attr = { 0 };
    char              recv_buf[DATA_BUF_SIZE];


    /* close all writing end */
    for (int i = 1; i <= index; i++) {
        close(sktpair[i - 1][1]);
    }

    if (NULL == qp_socket_create(&client)) {
        fprintf(stderr, "\nThread [%d] init client fail.", index);
        return -1;
    }

    if (!qp_socket_set_attr(&client, QP_SO_NOBLOCK, NULL, 0)) {
        return -1;
    }

    while (1) {

        if (NULL != qp_socket_accept(&listener, &client)) {

            if (qp_fd_is_valid(&client.socket)) {
                /* set no block */
                if (qp_socket_init_attr(&client)) {
                    /* add to epoll event and process */
                    qp_socket_recv(&client, recv_buf, DATA_BUF_SIZE, 0);
                }

//                qp_socket_close(&client);
            }
        }
    }

    qp_socket_destory(&client);
    qp_socket_destory(&listener);
    return 0;
}


struct  testcase_s {
    unsigned    testcase_count[TESTCASE_THREAD_NUM];
    pthread_t    thread_slot[TESTCASE_THREAD_NUM];
    bool           is_run;
};

void*
testcase_thread(void* arg) {
    struct testcase_s *cases = (struct testcase_s*) arg;
    int index = 0;
    for (; index < TESTCASE_THREAD_NUM; index++) {
        if (cases->thread_slot[index] == pthread_self()) {
            break;
        }
    }

    cases->testcase_count[index] = 0;
    qp_socket_t     connector;

    while(cases->is_run) {

        if (NULL != qp_socket_init(&connector, AF_INET, NULL, "127.0.0.1", LISTENER_PORT)) {

            if (QP_ERROR != qp_socket_connect(&connector)) {
                QP_ATOM_ADD(&cases->testcase_count[index], 1);
            }

            qp_socket_destory(&connector);
        }
    }

    return NULL;
}

qp_int_t
test_case()
{
    unsigned count = 0;
    struct testcase_s  cases;
    cases.is_run = true;

    for (int i = 0; i < TESTCASE_THREAD_NUM; i++) {
        pthread_create(&cases.thread_slot[i], NULL, testcase_thread, &cases);
        pthread_detach(cases.thread_slot[i]);
    }

    timeval   tver;

    while(cases.is_run) {
        tver.tv_sec = 1;
        tver.tv_usec = 0;
        select(0, NULL, NULL, NULL, &tver);
        count = 0;

        for (int j = 0; j < TESTCASE_THREAD_NUM; j++) {
            count += QP_ATOM_FETCH(&cases.testcase_count[j]);
            cases.testcase_count[j] = 0;
        }

        fprintf(stderr, "\n Current request: %u", count);
    }
    
    return 0;
}

