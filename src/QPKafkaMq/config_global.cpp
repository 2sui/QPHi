
#include "configuration.h"
#include <unistd.h>

const char* mq_config[] = {
    "client.id",
    "metadata.broker.list",
    "message.max.bytes",
    "receive.message.max.bytes",
    "metadata.request.timeout.ms",
    "topic.metadata.refresh.interval.ms",
    "topic.metadata.refresh.fast.cnt",
    "topic.metadata.refresh.fast.interval.ms",
    "topic.metadata.refresh.sparse",
    "debug",
    "socket.timeout.ms",
    "socket.send.buffer.bytes",
    "socket.receive.buffer.bytes",
    "socket.keepalive.enable",
    "socket.max.fails",
    "broker.address.ttl",
    "broker.address.family",
    "statistics.interval.ms",
    "error_cb",
    "stats_cb",
    "log_cb",
    "log_level",
    "socket_cb",
    "open_cb",
    "opaque",
    "internal.termination.signal",
    NULL
};

const char*  mq_consumer_config[] = {
    "queued.min.messages",
    "queued.max.messages.kbytes",
    "fetch.wait.max.ms",
    "fetch.message.max.bytes",
    "fetch.min.bytes",
    "fetch.error.backoff.ms",
    "group.id",
    NULL
};

const char* mq_producer_config[] = {
    "queue.buffering.max.messages",
    "queue.buffering.max.ms",
    "message.send.max.retries",
    "retry.backoff.ms",
    "compression.codec",
    "batch.num.messages",
    "delivery.report.only.error",
    "dr_cb",
    "dr_msg_cb",
    NULL
};

