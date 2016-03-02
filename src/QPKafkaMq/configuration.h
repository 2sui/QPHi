#ifndef CONFIGURATION_GLOBAL_H
#define CONFIGURATION_GLOBAL_H

#define  MQ_DEBUG

#ifdef MQ_DEBUG
#ifndef QP_DEBUG
#define QP_DEBUG
#endif
#endif

#define MQ_LOGOUT_LOG  QP_LOGOUT_LOG
#define MQ_LOGOUT_ERROR  QP_LOGOUT_ERROR



extern const char* mq_config[];
extern const char* mq_consumer_config[];
extern const char* mq_producer_config[];

#endif // CONFIGURATION_GLOBAL_H
