
/**
  * Copyright (C) 2sui.
  */


#ifndef QP_DEBUG_H
#define QP_DEBUG_H


//#define  QP_DEBUG /* uncomment it if need debug info */

#define LOGOUT_TO   stderr

#ifdef QP_DEBUG
#define QP_LOGOUT_FAIL(reason...) { \
    fprintf(LOGOUT_TO, "\n[FAIL] QP_Debug Failed at %s:%i:%s()\n\t", \
    __FILE__, __LINE__, __FUNCTION__);	\
    fprintf(LOGOUT_TO, reason);  \
    exit(1); \
}
#else
#define QP_LOGOUT_FAIL(reason...) ({exit(1);})
#endif


#ifdef QP_DEBUG
#define  QP_LOGOUT_ERROR(reason...) { \
    fprintf(LOGOUT_TO, "\n[ERROR] QP_Debug Error at %s:%i:%s()\n\t", \
    __FILE__, __LINE__, __FUNCTION__); \
    fprintf(LOGOUT_TO, reason); \
}
#else
#define QP_LOGOUT_ERROR(reason...) 
#endif


#ifdef QP_DEBUG
#define QP_LOGOUT_LOG(info...) { \
    fprintf(LOGOUT_TO, "\n[LOG] "); \
    fprintf(LOGOUT_TO, info); \
}
#else
#define QP_LOGOUT_LOG(info...) 
#endif


#endif // QP_DEBUG_H

