
#ifndef MQ_OBJ_H
#define MQ_OBJ_H

#include <qp_event.h>
#include <string>
#include <vector>
#include <exception>
#include "rdkafka.h"
#include "configuration.h"


#define    CONNECTION_POOL_SIZE              1024
#define    KAFKA_PRODUCER_WAIT_TIMEOUT       20
#define    MQ_VOLUME_MAX          16    /* max element number per object */
#define    MQ_TOPIC_VOLUME_MAX    16    /* max topic number per element */
#define    MQ_ERRBUF_SIZE         512
#define    MQ_GET_MEATADA_TIMEOUT 30      /* wait timeout for topic metadata */
#define    MQ_QUIT_TIMEOUT        (60)    /* wait timeout for poll when quit (second) */

typedef  qp_event_t               mq_event_t;
typedef  qp_event_fd_t            mq_event_fd_t;
typedef  qp_event_data_t          mq_event_data_t;

#define  mq_event_module_init     qp_event_init
#define  mq_event_module_destroy  qp_event_destroy
#define  mq_event_process         qp_event_tiktok
#define  MQ_SUCCESS               QP_EVENT_SUCCESS
#define  MQ_ERROR                 QP_EVENT_ERROR

#ifdef MQ_DEBUG
#define MQ_LOGOUT_REPORT(what...) { \
    fprintf(LOGOUT_TO, "\n[REPORT] "); \
    fprintf(LOGOUT_TO, (char*)what); \
}
#else
#define LOGOUT_REPORT(what...)
#endif


struct  topic_s {
    const struct rd_kafka_metadata  *metadata;
    rd_kafka_topic_t*           topic;
    rd_kafka_topic_conf_t*      topic_conf;
    rd_kafka_message_t*         messages;
    std::vector<int32_t>        partitions_list;  /* partitions in this topic */
    std::vector<size_t>         offset_list;   /* offset for consumer per partition */
};

typedef  struct topic_s         topic_t;


struct  mq_s {
    rd_kafka_t*                 kafka;
    rd_kafka_conf_t*            kafka_conf;
    std::vector<topic_t>        topic_list;

    size_t                      topic_volume;    /* max topic element num */
    size_t                      topic_last;    /* used topic element offset */
    char                        errstr[MQ_ERRBUF_SIZE];
};

typedef struct mq_s             mq_t;


class mq_obj
{
public:
    /**
     *  @param  [type]    producer or consumer.
     *  @param  [volume]  Max element number in object.
     *  @param  [topic_volume] Max topic number per element.
    */
    mq_obj(rd_kafka_type_t type = RD_KAFKA_PRODUCER,
           size_t volume = 1, size_t topic_volume = 1);
    ~mq_obj();    


    mq_t*
    mq_get_element(size_t index) 
    {return (o_mq_list.empty() ? NULL : &o_mq_list[index]);}

    /**
     * Set  global config info for kafka.
     *
     * @param [index] Element index.
     * @param [name] Configuration name.
     * @param [value] Configuration value.
     *
     * @return Return MQ_SUCCESS if success, otherwise return
     *                  MQ_ERROR.
     */
    int
    mq_set_conf(size_t index, const char* name, const char* value);

    /**
     * Set topic config info.
     *
     * @param [index] Element index.
     * @param [name] Configuration name.
     * @param [value] Configuration value.
     *
     * @return Return MQ_SUCCESS if success, otherwise return
     *                  MQ_ERROR.
     */
    int
    mq_set_topic_conf(size_t index, size_t topic_index, \
        const char* name, const char* value);



    /**
     * @brief mq_set_err_callback
     */
    void
    mq_set_err_callback(void (*err_callback)( rd_kafka_t* kafka,
        int err, const char* reason, void* opaque));

    /**
     * @brief mq_set_log_callback
     */
    void
    mq_set_log_callback(void (*log_callback)(const rd_kafka_t* kafka,
        int level, const char* fac, const char* buf));

    /**
     * @brief mq_set_stat_callback
     */
    void
    mq_set_stat_callback(int (*stats_callback)( rd_kafka_t* kafka,
        char* json, size_t json_size, void* opaque));

    /**
     * @brief mq_set_delivery_callback
     */
    void
    mq_set_delivery_callback(void (*delivery_callback)( rd_kafka_t* kafka,
        const rd_kafka_message_t* message, void* msg_opaque));

    /**
     * Setup kafka producers/consumers.
     *
     * @return MQ_SUCCESS if success, otherwise return MQ_ERROR.
     */
    int
    mq_setup();

    /**
     * @brief mq_add_broker
     * @param index
     * @param brokers
     * @return
     */
    int
    mq_add_broker(size_t index, const char* brokers);

    /**
     * @brief mq_add_topic
     *
     * @param index
     * @param topic
     *
     * @return
     */
    int
    mq_add_topic(size_t index, const char* topic);



    /**
     * Produce data.
     *
     * The data souce can be set with [payload] or [fd[]].
     * If [payload] is set the function will produce one data per calling,
     * And if [fd[]] is set , the function will produce data when any of those
     * fd is ready, until all of those fd is closed.[init] and [destory] is
     * the way that how to init and destory the elements struct mq_event_fd_t
     * that linked to fd[].
    */
    int
    mq_produce(size_t index,   /* element [index] */
        \
        /* those arguements are for produce data per message. */
        size_t  topic_index,    /* topic index of the element */
        int32_t partition,     /* partition of the topic that produce to */
        char*   payload,     size_t len, /* data source for single data */
        void*   key,         size_t keylen,  /* message key */
        \
        /* those arguements are for produce data from fd, e.g a socket */
        int     fd[],        size_t fdlen,    /* data source for fd*/
        int     listenfd[],  size_t listenfdlen, /* listening fd */
        int (*init)(mq_event_data_t*, \
            int (**)(int, qp_event_data_t*), int),
        int (*destroy)(mq_event_data_t*),
        \
        void* msg_opaque = NULL);

protected:


private:
    std::vector<mq_t>                   o_mq_list;    /* kafka object */
    rd_kafka_type_t                     o_type;       /* producer or consumer */
    mq_event_t*                         o_events;
    size_t                              o_volume;     /* max elements number */
    int                                 o_quit_timeout;

    /* error callback handler */
    void (*o_mq_err_cb)( rd_kafka_t* kafka,
        int err, const char* reason, void* opaque);

    /* log callback handler */
    void (*o_mq_log_cb)(const rd_kafka_t* kafka,
        int level, const char* fac, const char* buf);

    /* stats callback handler */
    int (*o_mq_stats_cb)( rd_kafka_t* kafka,
        char* json, size_t json_size, void* opaque);

    /* delivery callback handler */
    void (*o_mq_delivery_cb)( rd_kafka_t* kafka,
        const rd_kafka_message_t* message, void* msg_opaque);
};


#endif // MQ_OBJ_H
