
#ifndef ECALLBACK_H
#define ECALLBACK_H


#include  "mq_obj.h"

#define  MQ_HTTP_RESPON_200_HTML \
"<html>"\
"<head>"\
"<titile>MQ_Monitor</titile>"\
"</head>"\
"<body>"\
"<h1>Method : %s</h1>"\
"<h2>URL : %s</h2>"\
"<h2>Content-length: %lu</h2>"\
"<h2>Post data: %s</h2>"\
"<div>"\
"<form action=\"/\" method=\"post\">"\
"Topic: <input type=\"text\" name=\"topic\" />"\
"Partition: <input type=\"text\" name=\"partition\" />"\
"<input type=\"submit\" />"\
"</form>"\
"</div>"\
"</body>"\
"</html>"

#define  MQ_HTTP_RESPON_400_HTML \
"<html>"\
"<head>"\
"<titile>MQ_Monitor</titile>"\
"</head>"\
"<body>"\
"<h1>400 Not Found... T_T</h1>"\
"</body>"\
"</html>"

#define  MQ_HTTP_RESPON_500_HTML \
"<html>"\
"<head>"\
"<titile>MQ_Monitor</titile>"\
"</head>"\
"<body>"\
"<h1>500 Internal Server Error... T_T</h1>"\
"</body>"\
"</html>"

extern mq_obj*     mq_producer;

extern std::vector<int (*)(mq_event_data_t*,int (**)(int,qp_event_data_t*),int)>\
    mq_producer_event_init_list;

extern std::vector<int (*)(mq_event_data_t *)> \
    mq_producer_event_destroy_list;


int
producer_main(int argc, char* argv[]);

/**
 * Default event_fd_t init function.
 */
int
mq_event_fd_init_for_bk0(mq_event_data_t *efd, \
    int (**)(int, qp_event_data_t*), int fd);


/**
 * Default event_fd_t destory function.
 * @return
 */
int
mq_event_fd_destroy_for_bk0(mq_event_data_t *efd);


#endif // ECALLBACK_H
