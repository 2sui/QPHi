
/**
 * Copyright (C) 2sui.
 */


#ifndef QP_HTTP_PARSER_H
#define QP_HTTP_PARSER_H

#define  QP_HTTP_PARSER_DEBUG

#ifndef MQ_HTTP_PARSE_H
#define MQ_HTTP_PARSE_H
#endif

#ifdef QP_HTTP_PARSER_DEBUG
#ifndef QP_DEBUG
#define QP_DEBUG
#endif
#endif


#include <qp_core.h>
#include <http_parser.h>


#define  HTTP_TO_PARSE              HTTP_REQUEST
#ifndef HTTP_PARSER_STRICT
#define  HTTP_PARSER_STRICT         1  /* if 0 means make less check but run faster */
#endif
#ifndef HTTP_MAX_HEADER_SIZE
#define  HTTP_MAX_HEADER_SIZE       (80*1024) /* max allowed header size */
#endif

#define   QP_PARSER_METHED_DELETE       0
#define   QP_PARSER_METHED_GET          1
#define   QP_PARSER_METHED_HEAD         2
#define   QP_PARSER_METHED_POST         3
#define   QP_PARSER_METHED_PUT          4
#define   QP_PARSER_METHED_CONNECT      5
#define   QP_PARSER_METHED_OPTIONS      6
#define   QP_PARSER_METHED_TRACE        7
#define   QP_PARSER_METHED_COPY         8
#define   QP_PARSER_METHED_LOCK         9
#define   QP_PARSER_METHED_MKCOL        10
#define   QP_PARSER_METHED_MOVE         11
#define   QP_PARSER_METHED_PROPFIND     12
#define   QP_PARSER_METHED_PROPPATCH    13
#define   QP_PARSER_METHED_SEARCH       14
#define   QP_PARSER_METHED_UNLOCK       15
#define   QP_PARSER_METHED_BIND         16
#define   QP_PARSER_METHED_REBIND       17
#define   QP_PARSER_METHED_UNBIND       18
#define   QP_PARSER_METHED_ACL          19
#define   QP_PARSER_METHED_REPORT       21
#define   QP_PARSER_METHED_MKACTIVITY   21
#define   QP_PARSER_METHED_CHECKOUT     22
#define   QP_PARSER_METHED_MERGE        23
#define   QP_PARSER_METHED_M_SEARCH     24
#define   QP_PARSER_METHED_NOTIFY       25
#define   QP_PARSER_METHED_SUBSCRIBE    26
#define   QP_PARSER_METHED_UNSUBSCRIBE  27
#define   QP_PARSER_METHED_PATCH        28
#define   QP_PARSER_METHED_PURGE        29
#define   QP_PARSER_METHED_MKCALENDAR   30
#define   QP_PARSER_METHED_LINK         31
#define   QP_PARSER_METHED_UNLINK       32

#define   QP_PARSER_PARSING         1

#define  QP_HTTP_RESPON_200_STAT    0  /* HTTP responce 200 OK */
#define  QP_HTTP_RESPON_400_STAT    1  /* HTTP responce 400 Bad request */
#define  QP_HTTP_RESPON_500_STAT    2  /* HTTP responce 500 Internal error */

#define  QP_HTTP_RESPON_CLOSE       0  /* HTTP responce connection close */
#define  QP_HTTP_RESPON_KEEPALIVE   1  /* HTTP responce connection keep-alive */


#ifndef _BITS_UIO_H
struct iovec {
    void*  iov_base;
    size_t iov_len;
};
#endif


 /**
  * You can change the value of define below.
  */
#define  QP_HTTP_REQSTLINE_SIZE         1024 /* URL line string size */
#define  QP_HTTP_CONTENTLEN_CACHE_SIZE  64   /* content-length cache size */


struct  qp_http_request_s {
    /* fields */
    const qp_char_t*         method;
    qp_ushort_t              method_code;
    qp_ushort_t              chuncked;
    qp_char_t                URL[QP_HTTP_REQSTLINE_SIZE];
    size_t                   URL_offset;
    size_t                   content_len;
    const qp_char_t*         content;     /* body */

    /* stat */
    http_parser_settings*    user_setting;
    qp_int16_t               stat;   /* 0:nouse or parse done, 1: parsing */
};

typedef  struct qp_http_request_s    qp_http_request_t;

struct qp_http_responce_s {
    /* fields */
#define  QP_HTTP_RESPON_FIELD_NUM    5
    struct iovec             code;
    struct iovec             server;
    struct iovec             content_len;
    struct iovec             keepalive;
    struct iovec             body;

    /* stat */
    size_t                   iov_size;
    qp_char_t
    content_len_cache[QP_HTTP_CONTENTLEN_CACHE_SIZE];
};

typedef  struct qp_http_responce_s    qp_http_responce_t;


class qp_http_parse
{
public:
    /**
     * @param request_callbacks   User callback of http_parser_settings.
     * @param request_line_size    Max URL line size in request.
     */
    qp_http_parse(http_parser_settings* request_callbacks = NULL);
    
    virtual ~qp_http_parse();

    /**
     * Parse HTTP request.If return QP_SUCCESS that the request parse 
     * done, if return QP_PARSER_ERROR means some error happend when parsing, 
     * and return QP_PARSER_PARSING if it is parsing and waiting for more data.
     *
     * @param req       HTTP data.
     * @param req_size  HTTP data size.
     * @return
     */
    qp_int_t
    request_parse(const qp_char_t* req, size_t req_size);

    /**
     * Force complete parsing.
     */
    void
    request_parse_done();

    /**
     * Get parsing stat.
     *
     * @return QP_PARSER_ERROR / QP_PARSER_PARSING / QP_PARSER_SUCCESS.
     */
    qp_int_t
    request_parse_stat() { return request.stat; }

    /**
     * Get successfully parsed request.
     *
     * @return If parse success return parsed reques, otherwise return NULL.
     */
    const qp_http_request_t*
    request_get_parsed_request() {return !request.stat ? &request : NULL; }

    qp_int_t
    responce_set_code(qp_int_t code);

    qp_int_t
    responce_set_connection(qp_int_t connection);

    qp_int_t
    responce_set_body(const qp_char_t* body);

    size_t
    responce_get_frame(struct iovec** out);


private:
    /* parser part */
    http_parser_settings    parser_setting;
    http_parser             parser;

    /* request part */
    qp_http_request_t       request;

    /* responce part */
    qp_http_responce_t      responce;

};

#endif // QP_HTTP_PARSER
