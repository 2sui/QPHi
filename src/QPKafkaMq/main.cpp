
#include  <iostream>
#include  <qp_core.h>
#include  "ecallback.h"

using namespace std;


#define  BROKERS0        "192.168.11.201:9092"
#define  TOPICS          "develop_env"
#define  GROUPSIZE       1    /* group number in mq_producer */

vector<pthread_t>        plist;


void*
ProducerThread(void* arg) 
{
    mq_obj* producer = (mq_obj*)arg;
    size_t psize = plist.size();
    size_t index = 0;

    for (; index < psize; index++) {

        if (pthread_self() == plist[index]) {
            break;
        }
    }

    if (mq_producer_event_init_list.size() <= index
        || mq_producer_event_destroy_list.size() <= index)
    {
        pthread_exit(NULL);
        return NULL;
    }

    qp_socket_t socket;

    if (NULL == qp_socket_init(&socket, (qp_socket_type_t)AF_INET, \
        (qp_socket_sock_type_t)SOCK_STREAM, 
        NULL, "0.0.0.0", 8888 + index, true)) 
    {
        pthread_exit(NULL);
        return NULL;
    }

    qp_socket_set_attr(&socket, QP_SO_NOBLOCK, NULL, 0);
    qp_socket_set_attr(&socket, QP_SO_REUSEADDR, NULL, 0);
    qp_socket_reinit_attr(&socket, NULL);
    qp_socket_listen(&socket);

    MQ_LOGOUT_LOG("Producer [%lu] start...", index);

    producer->mq_produce(index, 0, 0, NULL, 0, NULL, 0, NULL , \
        CONNECTION_POOL_SIZE, (int*)&(socket.socket.fd), 1, \
        (mq_producer_event_init_list)[index], \
        (mq_producer_event_destroy_list)[index], NULL);

    qp_socket_destroy(&socket, false);
    MQ_LOGOUT_LOG("Producer [%lu] stop...", index);
    pthread_exit(NULL);
    return NULL;
}


int
producer_main(int argc, char* argv[])
{
    rd_kafka_type_t               type = RD_KAFKA_PRODUCER;
    std::vector<const char*>      topic;
    std::vector<const char*>      brokers;
    size_t                        group_size = GROUPSIZE;

    topic.push_back(TOPICS);
    
    brokers.push_back(BROKERS0);
    
    try {
        mq_producer = new mq_obj(type, group_size, topic.size());

    } catch (exception()) {
        MQ_LOGOUT_ERROR("Producer create fail.");
        return 1;
    }


    MQ_LOGOUT_LOG("Setting configuration...");

    for (size_t i = 0; i < group_size; i++) {
        mq_producer->mq_set_conf(i, "queue.buffering.max.messages", "500000");
        mq_producer->mq_set_conf(i, "message.send.max.retries", "3");
        mq_producer->mq_set_conf(i, "retry.backoff.ms", "500");
        mq_producer->mq_set_conf(i, "queue.buffering.max.messages", "500000");
        mq_producer->mq_set_conf(i, "queue.buffering.max.messages", "500000");

        if (MQ_SUCCESS != mq_producer->mq_setup()) {
            MQ_LOGOUT_ERROR("Setup index[%lu] fail.", i);
            goto end;
        }

        for (size_t j = 0; j < brokers.size(); j++) {
            
            if (MQ_SUCCESS != mq_producer->mq_add_broker(i, brokers[j])) {
                MQ_LOGOUT_ERROR("[%lu]Add broker : [%s] fail .", i, brokers[j]);
                goto end;
            }
            
            MQ_LOGOUT_LOG("[%lu]Add brokers : [%s].", i, brokers[j]);
        }
        
        for (size_t j = 0; j < topic.size(); j++) {
            mq_producer->mq_set_topic_conf(i,j,"produce.offset.report", "true");
            
            if (MQ_SUCCESS != mq_producer->mq_add_topic(i, topic[j])) {
                MQ_LOGOUT_ERROR("[%lu]Add topic : [%s] fail.", i, topic[j]);
                goto end;
            }
            
            MQ_LOGOUT_LOG("[%lu]Add topic : [%s].", i, topic[j]);
        }

    
        mq_producer_event_init_list.push_back(mq_event_fd_init_for_bk0);
        mq_producer_event_destroy_list.push_back(mq_event_fd_destroy_for_bk0);
    }

    /* create process */
    for (size_t i = 0; i < group_size; i++) {
        plist.push_back(0);
        pthread_create(&plist[i], NULL , ProducerThread, mq_producer);
    }

    for(size_t i = 0; i < group_size; i++) {

        if (plist[i] > 0) {
            pthread_join(plist[i], NULL);
        }
    }

end:
    MQ_LOGOUT_LOG("Exit.");
    delete mq_producer;
    return 0;
}

#ifdef DEVELOP

int
main(int argc, char* argv[])
{
    return producer_main(argc, argv);
}

#endif
