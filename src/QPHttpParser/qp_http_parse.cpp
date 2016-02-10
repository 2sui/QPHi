
/**
 * Copyright (C) 2sui.
 */


#include <qp_http_parse.h>
#include <exception>


static const qp_char_t* qp_http_parse_responce_code[] = {
    #define  QP_HTTP_RESPON_200_STAT_LEN    17
    "HTTP/1.1 200 OK\r\n",
    #define  QP_HTTP_RESPON_400_STAT_LEN    26
    "HTTP/1.1 400 Bad Request\r\n",
    #define  QP_HTTP_RESPON_500_STAT_LEN    36
    "HTTP/1.1 500 Internal Server Error\r\n",
    NULL
};


#define  QP_HTTP_RESPON_SERVER_LEN    14
static const qp_char_t*  qp_http_parse_responce_server = \
    "server: QPHi\r\n";


static const  qp_char_t* qp_http_parse_responce_keepalive[] = {
    #define QP_HTTP_RESPON_CLOSE_LEN    21
    "connection: close\r\n\r\n",
    #define  QP_HTTP_RESPON_KEEPALIVE_LEN    26
    "connection: keep-alive\r\n\r\n",
    NULL
};


qp_int_t
request_on_message_begin(http_parser* parser)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);
    request->method = NULL;
    request->method_code = 0xff00;
    request->chuncked = 0;
    request->URL_offset = 0;
    request->content_len = 0xffffffffffffffff;
    request->content = NULL;
    request->stat = QP_PARSER_PARSING;

    if (request->user_setting) {

        return (NULL == request->user_setting->on_message_begin) ? \
            QP_SUCCESS : request->user_setting->on_message_begin(parser);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_url(http_parser* parser, const char* at, size_t length)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);

    if ((request->URL_offset + length) > QP_HTTP_REQSTLINE_SIZE) {
        QP_LOGOUT_LOG("[qp_http_parse] Request line too long [current: %d].", \
            QP_HTTP_REQSTLINE_SIZE);
        return QP_ERROR;
    }

    /* overflow */
    memcpy(request->URL + request->URL_offset, at, length);
    request->URL_offset += length;
    request->URL[request->URL_offset] = 0;

    if (request->user_setting) {

        return (NULL == request->user_setting->on_url) ? \
            QP_SUCCESS : \
            request->user_setting->on_url(parser, request->URL, \
            request->URL_offset);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_status(http_parser* parser, const char* at, size_t length)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);

    if (request->user_setting) {

        return (NULL == request->user_setting->on_status) ? \
            QP_SUCCESS : \
            request->user_setting->on_status(parser, at, length);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_header_field(http_parser* parser, const char* at, size_t length)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);

    /* Content-Length */
    if (('C' == at[0] || 'c' == at[0])
        && ('O' == at[1] || 'o' == at[1])
        && ('N' == at[2] || 'n' == at[2])
        && ('T' == at[3] || 't' == at[3])
        && ('E' == at[4] || 'e' == at[4])
        && ('N' == at[5] || 'n' == at[5])
        && ('T' == at[6] || 't' == at[6])
        && ('-' == at[7] || '-' == at[7])
        && ('L' == at[8] || 'l' == at[8])
        && ('E' == at[9] || 'e' == at[9])
        && ('N' == at[10] || 'n' == at[10])
        && ('G' == at[11] || 'g' == at[11])
        && ('T' == at[12] || 't' == at[12])
        && ('H' == at[13] || 'h' == at[13]))
    {
        request->content_len = 0;
    }

    if (request->user_setting) {

        return (NULL == request->user_setting->on_header_field) ? \
            QP_SUCCESS : \
            request->user_setting->on_header_field(parser, at, length);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_header_value(http_parser* parser, const char* at, size_t length)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);

    if (0 == request->content_len) {

        /* range of content too large */
        if (length > QP_HTTP_CONTENTLEN_CACHE_SIZE) {
            QP_LOGOUT_LOG("[qp_http_parse] Request content-length too long"\
                " [current: %u].", QP_HTTP_CONTENTLEN_CACHE_SIZE);
            return QP_ERROR;
        }

        for ( ; length; length--, at++) {
            request->content_len = (request->content_len * 10) + at[0] - '0';
        }
    }

    if (request->user_setting) {

        return (NULL == request->user_setting->on_header_value) ? \
            QP_SUCCESS : \
            request->user_setting->on_header_value(parser, at, length);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_headers_complete(http_parser* parser)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);
    request->method_code = parser->method;
    request->method = http_method_str((http_method)(request->method_code));

    if (request->user_setting) {

        return (NULL == request->user_setting->on_headers_complete) ? \
            QP_SUCCESS : \
            request->user_setting->on_headers_complete(parser);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_body(http_parser* parser, const char* at, size_t length)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);

    request->content = at;

    if (request->user_setting) {

        return (NULL == request->user_setting->on_body) ? \
            QP_SUCCESS : \
            request->user_setting->on_body(parser, at, length);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_message_complete(http_parser* parser)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);
    request->stat = QP_SUCCESS;

    if (request->user_setting) {

        return (NULL == request->user_setting->on_message_begin) ? \
            QP_SUCCESS : request->user_setting->on_message_begin(parser);
    }

    return QP_SUCCESS;
}

qp_int_t
request_on_chunk_header(http_parser* parser)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);
    
    if (request->user_setting) {

        return (NULL == request->user_setting->on_chunk_header) ? \
            QP_SUCCESS : request->user_setting->on_chunk_header(parser);
    }
    
    return QP_SUCCESS;
}

qp_int_t
request_on_chunk_complete(http_parser* parser)
{
    qp_http_request_t* request = (qp_http_request_t*)(parser->data);
    
    if (request->user_setting) {

        return (NULL == request->user_setting->on_chunk_complete) ? \
            QP_SUCCESS : request->user_setting->on_chunk_complete(parser);
    }
    return QP_SUCCESS;
}


qp_http_parse::qp_http_parse(http_parser_settings *request_callbacks)
{
    memset(&request, 0, sizeof(qp_http_request_t));
    memset(&responce, 0, sizeof(qp_http_responce_t));
    request.user_setting = request_callbacks;
    responce.server.iov_base = \
        (void*)qp_http_parse_responce_server;
    responce.server.iov_len = QP_HTTP_RESPON_SERVER_LEN;
    responce.content_len.iov_base = (void*)responce.content_len_cache;
    responce.body.iov_base = NULL;
    responce.body.iov_len = 0;
    responce.iov_size = QP_HTTP_RESPON_FIELD_NUM;

    responce_set_code(QP_HTTP_RESPON_200_STAT);
    responce_set_connection(QP_HTTP_RESPON_CLOSE);
    responce_set_body(NULL);

    parser.data = &request;
    parser_setting.on_message_begin = request_on_message_begin;
    parser_setting.on_url = request_on_url;
    parser_setting.on_status = request_on_status;
    parser_setting.on_header_field = request_on_header_field;
    parser_setting.on_header_value = request_on_header_value;
    parser_setting.on_headers_complete = request_on_headers_complete;
    parser_setting.on_body = request_on_body;
    parser_setting.on_message_complete = request_on_message_complete;
    parser_setting.on_chunk_header = request_on_chunk_header;
    parser_setting.on_chunk_complete = request_on_chunk_complete;
}

qp_http_parse::~qp_http_parse()
{
}

qp_int_t
qp_http_parse::request_parse(const char *req, size_t req_size)
{
    switch (request.stat) {

    case QP_SUCCESS: {
        http_parser_init(&parser, HTTP_TO_PARSE);
        parser.data = &request;

        if (req_size != http_parser_execute(&parser, &parser_setting,
            req, req_size))
        {
            request_parse_done();
            return QP_ERROR;
        }

        return request.stat;
    }
        break;

    case QP_PARSER_PARSING: {

        if (req_size != http_parser_execute(&parser, &parser_setting,
            req, req_size))
        {
            request_parse_done();
            return QP_ERROR;
        }

        return request.stat;
    }
        break;

    default:{
        request_parse_done();
        QP_LOGOUT_LOG("[qp_http_parse] Unknow parsing stat [%u].", request.stat);
    }
        break;
    }

    return QP_ERROR;
}

void
qp_http_parse::request_parse_done()
{
    request.stat = QP_SUCCESS;
    http_parser_execute(&parser, &parser_setting, NULL, 0);
}

qp_int_t
qp_http_parse::responce_set_code(int code)
{
    switch (code) {

    case QP_HTTP_RESPON_200_STAT: {
        responce.code.iov_base = \
            (void*)qp_http_parse_responce_code[QP_HTTP_RESPON_200_STAT];
        responce.code.iov_len = QP_HTTP_RESPON_200_STAT_LEN;
    }
        break;

    case QP_HTTP_RESPON_400_STAT: {
        responce.code.iov_base = \
            (void*)qp_http_parse_responce_code[QP_HTTP_RESPON_400_STAT];
        responce.code.iov_len = QP_HTTP_RESPON_400_STAT_LEN;
    }
        break;

    case QP_HTTP_RESPON_500_STAT: {
        responce.code.iov_base = \
            (void*)qp_http_parse_responce_code[QP_HTTP_RESPON_500_STAT];
        responce.code.iov_len = QP_HTTP_RESPON_500_STAT_LEN;
    }
        break;
        
    default: {
        QP_LOGOUT_LOG("[qp_http_parse] Unknow responce code field.");
        return QP_ERROR;
    }
        break;
    }

    return QP_SUCCESS;
}

qp_int_t
qp_http_parse::responce_set_connection(int connection)
{

    switch (connection) {

    case QP_HTTP_RESPON_CLOSE: {
        responce.keepalive.iov_base = \
            (void*)qp_http_parse_responce_keepalive[QP_HTTP_RESPON_CLOSE];
        responce.keepalive.iov_len = QP_HTTP_RESPON_CLOSE_LEN;
    }
        break;

    case QP_HTTP_RESPON_KEEPALIVE: {
        responce.keepalive.iov_base = \
            (void*)qp_http_parse_responce_keepalive[QP_HTTP_RESPON_KEEPALIVE];
        responce.keepalive.iov_len = QP_HTTP_RESPON_KEEPALIVE_LEN;
    }
        break;

    default: {
        QP_LOGOUT_LOG("[qp_http_parse] Unknow responce connection field.");
        return QP_ERROR;
    }
        break;
    }

    return QP_SUCCESS;
}

qp_int_t
qp_http_parse::responce_set_body(const char *body)
{
    if (body) {
        responce.body.iov_base = (void*)body;
        responce.body.iov_len = strlen(body);

    } else {
        responce.body.iov_base = NULL;
        responce.body.iov_len = 0;
    }

    sprintf(responce.content_len_cache, "Content-Length: %lu\r\n",
            responce.body.iov_len);
    /* overflow */
    responce.content_len_cache[QP_HTTP_CONTENTLEN_CACHE_SIZE - 1] = 0;
    responce.content_len.iov_len = strlen(responce.content_len_cache);
    return QP_SUCCESS;
}

size_t
qp_http_parse::responce_get_frame(struct iovec** out)
{
    *out = (struct iovec*)&responce;
    return responce.iov_size;
}
