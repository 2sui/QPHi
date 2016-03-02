
#include "qp_http_parse.h"
#include "ecallback.h"


#define  MQ_EVENT_RECV_BUF_SIZE            (4*1024)
#define  MQ_EVENT_SEND_BUF_SIZE            (4*1024)


struct  mq_index_s {
    mq_t*                     kafka;
    void*                     key;
    size_t                    keylen;
    int                       topic_index;
    int                       partition_index;
    qp_http_parse             parser;
};

typedef  struct mq_index_s    mq_index_t;


/**
 * Defination of global variables.
 */

mq_obj*                       mq_producer = NULL;

std::vector<int (*)(mq_event_data_t *, int (**)(int, qp_event_data_t*), int)>\
    mq_producer_event_init_list;

std::vector<int (*)(mq_event_data_t *)>\
    mq_producer_event_destroy_list;


/**
  * Event process callback.
 */
int
mq_producer_event_precess(int fd, qp_event_data_t* data);


/**
 * There are callbacks for event.  Only mq_event_fd_init_for_brx and
 * mq_event_fd_destory_for_bkx are public.
 */

int
mq_event_fd_init_for_bk0(mq_event_data_t *efd,
    int (**do_myself)(int, qp_event_data_t*), int fd)
{
    /* set event callback */
    *do_myself = mq_producer_event_precess;

    efd->read_max = MQ_EVENT_RECV_BUF_SIZE;
    efd->readbuf = new char[efd->read_max];

    efd->write_max = MQ_EVENT_SEND_BUF_SIZE;
    efd->writebuf = new char[efd->write_max];

    /* set for kafka and http-parser */
    mq_index_t* index = new mq_index_t;
    efd->data = index;

    /* debug */
    if (!efd->readbuf || !efd->writebuf || !efd->data) {
        throw std::exception();
    }


    index->kafka = mq_producer->mq_get_element(0);
    index->key = NULL;
    index->keylen = 0;
    index->topic_index = 0;
    index->partition_index = 0;
    efd->writeiovec_size = index->parser.responce_get_frame(&(efd->writeiovec));

    return MQ_SUCCESS;
}


int
mq_event_fd_destroy_for_bk0(mq_event_data_t *efd)
{
    if (efd->data) {
        delete (mq_index_t*)(efd->data);
        efd->data = NULL;
    }

    if (efd->readbuf) {
        delete[] efd->readbuf;
        efd->readbuf  = NULL;
    }

    if (efd->writebuf) {
        delete[] efd->writebuf;
        efd->writebuf = NULL;
    }

    return MQ_SUCCESS;
}


/**
 * Main event process func.
 */
int
mq_producer_event_precess(int fd, qp_event_data_t* data)
{

    mq_index_t* mq = (mq_index_t*)(data->data);

    if (1 > fd) {

        if (QP_PARSER_SUCCESS != mq->parser.request_parse_stat()) {
            mq->parser.request_parse_done();
            MQ_LOGOUT_LOG("Parser close.");
        }

        return MQ_ERROR;
    }

    if (data->read_offset > 0) {

       /**
        * We use [http_parser.data] (also point to mq_http_filed_t) to 
        * differentiate the new connection and the old one. If [data.stat] is 
        * MQ_HTTP_PARSE_STAT_NOUSE  means the connection is a new one, 
        * if [data.stat] is MQ_HTTP_PARSE_STAT_PARSING means connection is 
        * parsing, if [data.stat] is MQ_HTTP_PARSE_STAT_DONE means we need 
        * respond with 200 ok, and if other (for example, 
        * MQ_HTTP_PARSE_STAT_ERROR) we need to respond with 400 bad request.
        */

        /* if recv data we need to parse  */
        if (data->read_start > 0) {
            data->read_start = 0;

            switch (mq->parser.request_parse(data->readbuf, data->read_offset)){

            /* parse success */
            case QP_PARSER_SUCCESS: {

               /**
                 * There is an issuse that if the body is chunked, the message
                 * produced also be chuncked.
                 */

               /* produce message */
                if (MQ_ERROR == \
                    rd_kafka_produce(mq->kafka->topic_list[mq->topic_index].topic,
                    mq->partition_index, RD_KAFKA_MSG_F_COPY,
                    data->readbuf, data->read_offset,
                    (const void*)(mq->key), mq->keylen, NULL))
                {
                    rd_kafka_poll(mq->kafka->kafka, 5);

                   /* error happen */
                    if (ENOBUFS != errno) {
                        /* kafka error, quit */
                        mq->parser.responce_set_code(QP_HTTP_RESPON_500_STAT);
                        mq->parser.responce_set_connection(QP_HTTP_RESPON_CLOSE);
                        mq->parser.responce_set_body(MQ_HTTP_RESPON_500_HTML);
                        return EPOLLOUT;
                    }
                }

                rd_kafka_poll(mq->kafka->kafka, 0);
                mq->parser.responce_set_code(QP_HTTP_RESPON_200_STAT);
                mq->parser.responce_set_connection(QP_HTTP_RESPON_CLOSE);
                sprintf(data->writebuf, MQ_HTTP_RESPON_200_HTML,
                    mq->parser.request_get_parsed_request()->method,
                    mq->parser.request_get_parsed_request()->URL,
                    mq->parser.request_get_parsed_request()->content ? \
                        mq->parser.request_get_parsed_request()->content_len : 0,
                    mq->parser.request_get_parsed_request()->content);
                    mq->parser.responce_set_body(data->writebuf);
                return EPOLLOUT;
            }
                break;

               /* parse error */
            case QP_PARSER_ERROR: {
                MQ_LOGOUT_LOG("Parser error.");

                mq->parser.responce_set_code(QP_HTTP_RESPON_400_STAT);
                mq->parser.responce_set_connection(QP_HTTP_RESPON_CLOSE);
                mq->parser.responce_set_body(MQ_HTTP_RESPON_400_HTML);
            }
                break;

            default:
                break;
            }

           /* just close the connection */
        } else {
            mq->parser.request_parse_done();
            return MQ_ERROR;
        }
    }

    return MQ_SUCCESS;
}
