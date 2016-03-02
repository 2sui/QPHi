
#include "mq_obj.h"

#ifndef MQ_DEBUG
#define BROKER_TEST
#endif


static void*
mq_wait_callback(void *arg)
{
    mq_t* kafka = (mq_t*)arg;
    rd_kafka_poll(kafka->kafka, KAFKA_PRODUCER_WAIT_TIMEOUT);
    return NULL;
}


/* error callback */
static void
mq_err_callback( rd_kafka_t* kafka, int err, const char* reason, void* opaque)
{
    MQ_LOGOUT_ERROR("%s : %s", rd_kafka_err2str((rd_kafka_resp_err_t)err), reason);
}


/* log info callback */
static void
mq_log_callback(const rd_kafka_t* kafka,
    int level, const char* fac, const char* buf)
{
    MQ_LOGOUT_LOG("%s : %s", fac, buf);
}


/* statistics callback */
static int
mq_stats_callback(rd_kafka_t* kafka, char* json, size_t json_size, void* opaque)
{
    return 0;
}


/* delivery report callback  */
static  void
mq_delivery_callback(rd_kafka_t* kafka,
    const rd_kafka_message_t* message, void* msg_opaque)
{
    MQ_LOGOUT_REPORT("Send to Partition: %d \tPayload: %s .",
        message->partition, (char*)message->payload);
}


mq_obj::mq_obj(rd_kafka_type_t type, size_t volume, size_t topic_volume) :
    o_type(type),
    o_quit_timeout(MQ_QUIT_TIMEOUT),
    o_mq_err_cb(NULL),
    o_mq_log_cb(NULL),
    o_mq_stats_cb(NULL),
    o_mq_delivery_cb(NULL)
{
    if ((1 > volume) || (MQ_VOLUME_MAX < volume)
         || (1 > topic_volume) || (MQ_TOPIC_VOLUME_MAX < topic_volume)) 
    {
        throw std::exception();
    }

    mq_t         mq;
    o_volume = 0;
    mq.kafka = NULL;
    mq.kafka_conf = NULL;
    mq.topic_volume = topic_volume;
    mq.topic_last = 0;

    /* set callback handler */
    o_mq_err_cb = mq_err_callback;
    o_mq_log_cb = mq_log_callback;
    o_mq_delivery_cb = mq_delivery_callback;
    o_mq_stats_cb = mq_stats_callback;

    /* create [o_volume] elements */
    for (size_t i = 0; i < volume; i++) {
        o_mq_list.push_back(mq);
    }

    /* create kafka conf and topic conf */
    for (; o_volume < volume; o_volume++) {
        /* create conf */
        o_mq_list[o_volume].kafka_conf = rd_kafka_conf_new();
        
        if (NULL == o_mq_list[o_volume].kafka_conf) {
            throw std::exception();
        }
        
        rd_kafka_conf_set_opaque(o_mq_list[o_volume].kafka_conf, \
            &o_mq_list[o_volume]);

        topic_t     topic;
        topic.topic = NULL;
        topic.topic_conf = NULL;
        topic.metadata = NULL;
        topic.messages = NULL;
        o_mq_list[o_volume].topic_list.clear();

        /*  create topic element */
        for (size_t j = 0; j < o_mq_list[o_volume].topic_volume; j++) {
            o_mq_list[o_volume].topic_list.push_back(topic);
            
            /* create topic conf */
            o_mq_list[o_volume].topic_list[j].topic_conf = \
                rd_kafka_topic_conf_new();
        
            if (NULL == o_mq_list[o_volume].topic_list[j].topic_conf) {
                throw std::exception();
            }
        
            rd_kafka_topic_conf_set_opaque(\
                o_mq_list[o_volume].topic_list[j].topic_conf, \
                &o_mq_list[o_volume]);
        }
    }
}


mq_obj::~mq_obj()
{
    for (size_t i = 0; i < o_volume; i++) {
        o_quit_timeout = MQ_QUIT_TIMEOUT;

        while (o_quit_timeout 
              && (0 < rd_kafka_poll(o_mq_list[i].kafka, 1000))) 
        {
            o_quit_timeout--;
        }

        int outq = rd_kafka_outq_len(o_mq_list[i].kafka);
        MQ_LOGOUT_LOG("There is %u messages in element [%lu]", outq, i);

        /* destory topics */
        for (size_t j = 0; j < o_mq_list[i].topic_volume; j++) {

            if (o_mq_list[i].topic_list[j].metadata) {
                rd_kafka_metadata_destroy(o_mq_list[i].topic_list[j].metadata);
                o_mq_list[i].topic_list[j].metadata = NULL;
            }
            
            if (o_mq_list[i].topic_list[j].topic) {
                rd_kafka_topic_destroy(o_mq_list[i].topic_list[j].topic);
                
            } else {
                rd_kafka_topic_conf_destroy(\
                    o_mq_list[i].topic_list[j].topic_conf);
            }
        }

        if (NULL != o_mq_list[i].kafka) {
            rd_kafka_destroy(o_mq_list[i].kafka);
        }
    }

    rd_kafka_wait_destroyed(2000);
}


int
mq_obj::mq_set_conf(size_t index, const char *name, const char *value)
{
    return (RD_KAFKA_CONF_OK == \
        rd_kafka_conf_set(o_mq_list[index].kafka_conf, name, \
        value, o_mq_list[index].errstr, MQ_ERRBUF_SIZE)) ? \
        MQ_SUCCESS : MQ_ERROR;
}


int
mq_obj::mq_set_topic_conf(size_t index, size_t topic_index, \
    const char *name, const char *value)
{
    return (RD_KAFKA_CONF_OK == \
        rd_kafka_topic_conf_set(\
            o_mq_list[index].topic_list[topic_index].topic_conf, name,\
            value, o_mq_list[index].errstr, MQ_ERRBUF_SIZE)) ? \
            MQ_SUCCESS : MQ_ERROR;
}


void
mq_obj::mq_set_err_callback(void (*err_callback)( rd_kafka_t *, int, \
    const char *, void *))
{
    if (err_callback) {
        o_mq_err_cb = err_callback;
    }
}


void
mq_obj::mq_set_log_callback(void (*log_callback)(const rd_kafka_t *, int, \
    const char *, const char *))
{
    if (log_callback) {
        o_mq_log_cb = log_callback;
    }
}


void
mq_obj::mq_set_stat_callback(int (*stats_callback)(rd_kafka_t *, char *, \
    size_t, void *))
{
    if (stats_callback) {
        o_mq_stats_cb = stats_callback;
    }
}


void
mq_obj::mq_set_delivery_callback(void (*delivery_callback)(rd_kafka_t *, \
    const rd_kafka_message_t *, void *))
{
    if (delivery_callback) {
        o_mq_delivery_cb = delivery_callback;
    }
}


int
mq_obj::mq_setup()
{
    for (size_t i = 0; i < o_volume; i++) {

        if (NULL == o_mq_list[i].kafka) {

            /* set error callback */
            rd_kafka_conf_set_error_cb(o_mq_list[i].kafka_conf, o_mq_err_cb);
            /* set log callback */
            rd_kafka_conf_set_log_cb(o_mq_list[i].kafka_conf, o_mq_log_cb);
            /* set delivery callback */
            rd_kafka_conf_set_dr_msg_cb(o_mq_list[i].kafka_conf,
                o_mq_delivery_cb);
            /* set stats callback */
            rd_kafka_conf_set_stats_cb(o_mq_list[i].kafka_conf,o_mq_stats_cb);

            o_mq_list[i].kafka = \
                rd_kafka_new(o_type, o_mq_list[i].kafka_conf, \
                o_mq_list[i].errstr, MQ_ERRBUF_SIZE);

            if (NULL == o_mq_list[i].kafka) {
                MQ_LOGOUT_ERROR("Kakfa [%lu] init fail.", i);
                continue;
            }

#ifdef MQ_DEBUG
            rd_kafka_set_log_level(o_mq_list[i].kafka, 7);
#endif
        }
    }

    return MQ_SUCCESS;
}


int
mq_obj::mq_add_broker(size_t index, const char *brokers)
{
    if ((NULL != brokers)
         && (index < o_volume)
         && (NULL != o_mq_list[index].kafka))
    {
        if (0 < rd_kafka_brokers_add(o_mq_list[index].kafka, brokers)) {
            return MQ_SUCCESS;
        }
    }

    return MQ_ERROR;
}


int
mq_obj::mq_add_topic(size_t index, const char *topic)
{
    if ((NULL != topic)
        && (index < o_volume)
        && (NULL != o_mq_list[index].kafka)
        && (NULL ==\
        o_mq_list[index].topic_list[o_mq_list[index].topic_last].topic))
    {
         int offset = o_mq_list[index].topic_last;
         o_mq_list[index].topic_list[offset].topic = \
            rd_kafka_topic_new(o_mq_list[index].kafka, topic, \
            o_mq_list[index].topic_list[offset].topic_conf);

         if (NULL == o_mq_list[index].topic_list[offset].topic) {
             return MQ_ERROR;
         }

         if (RD_KAFKA_RESP_ERR_NO_ERROR != \
            rd_kafka_metadata(o_mq_list[index].kafka, 0, \
            o_mq_list[index].topic_list[offset].topic, \
            &(o_mq_list[index].topic_list[offset].metadata), \
            MQ_GET_MEATADA_TIMEOUT * 1000))
         {
             o_mq_list[index].topic_list[offset].metadata = NULL;
             MQ_LOGOUT_LOG("[%lu]Topic: %s connect timeout.", index, topic);
         }
        

         if ((++o_mq_list[index].topic_last) >= \
            o_mq_list[index].topic_volume) 
         {
            o_mq_list[index].topic_last--;
         }

        return MQ_SUCCESS;
    }

    return MQ_ERROR;
}


int
mq_obj::mq_produce(size_t index, size_t topic_index, int32_t partition,
    char *payload, size_t len, 
    void *key,     size_t keylen,
    int fd[],      size_t fdlen, 
    int listenfd[],size_t listenfdlen,
    int (*init)(mq_event_data_t *, int (**)(int, qp_event_data_t*), int),
    int (*destory)(mq_event_data_t *), 
    void *msg_opaque)
{
    if ((RD_KAFKA_PRODUCER != o_type)
        || (o_volume <= index))
    {
        return MQ_ERROR;
    }

    if (topic_index > o_mq_list[index].topic_last) {
            return MQ_ERROR;
    }

    /*
     * Produce one piece data.
    */

    if ((NULL != payload) && (0 < len)) {
        
        /* check partition available */
        if (0 == rd_kafka_topic_partition_available(\
            o_mq_list[index].topic_list[topic_index].topic, partition)) 
        {
            return MQ_ERROR;
        }
        
        /* single data */
        if (MQ_ERROR == \
            rd_kafka_produce(o_mq_list[index].topic_list[topic_index].topic,
            partition, RD_KAFKA_MSG_F_COPY, payload, len, \
            key, keylen, msg_opaque))
        {
            rd_kafka_poll(o_mq_list[index].kafka, 5);
            return MQ_ERROR;
        }

        rd_kafka_poll(o_mq_list[index].kafka, 0);
        return MQ_SUCCESS;
    }


    /*
     * Produce data when fd event happen.
    */

    if (((NULL == fd) || (1 > fdlen))
        && ((NULL == listenfd) || (1 > listenfdlen))) 
    {
        return MQ_ERROR;
    }

    /* create event queue */
    mq_event_t     efd;
    int            run = 1;

    /*  init event module */
    if (NULL == mq_event_module_init(&efd, 0, listenfd, listenfdlen,\
        NULL, fdlen, true, true, init, destory, \
        mq_wait_callback, &o_mq_list[index]))
    {
        return MQ_ERROR;
    }

    mq_event_process(&efd, CONNECTION_POOL_SIZE, &run);

    mq_event_module_destroy(&efd);
    return MQ_SUCCESS;
}

