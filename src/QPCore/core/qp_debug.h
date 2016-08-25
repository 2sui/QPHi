/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QP_DEBUG_H
#define QP_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif


//#define  QP_DEBUG /* uncomment it if need debug info */

#define LOGOUT_TO   stderr

#ifdef QP_DEBUG
#define qp_debug_fail(reason...) { \
    fprintf(LOGOUT_TO, "\n[FAIL] QP_Debug Failed at %s:%i:%s()\n\t", \
    __FILE__, __LINE__, __FUNCTION__);	\
    fprintf(LOGOUT_TO, reason);  \
    exit(1); \
}
#else
#define qp_debug_fail(reason...) ({exit(1);})
#endif


#ifdef QP_DEBUG
#define  qp_debug_error(reason...) { \
    fprintf(LOGOUT_TO, "\n[ERROR] QP_Debug Error at %s:%i:%s()\n\t", \
    __FILE__, __LINE__, __FUNCTION__); \
    fprintf(LOGOUT_TO, reason); \
}
#else
#define qp_debug_error(reason...) 
#endif


#ifdef QP_DEBUG
#define qp_debug_info(info...) { \
    fprintf(LOGOUT_TO, "\n[LOG] "); \
    fprintf(LOGOUT_TO, info); \
}
#else
#define qp_debug_info(info...) 
#endif

#ifdef __cplusplus
}
#endif

#endif /* QP_DEBUG_H */
