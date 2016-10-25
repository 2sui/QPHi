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


#ifndef QP_POOL_H
#define QP_POOL_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"

    
typedef struct qp_pool_elm_s*            qp_pool_elm_t;
typedef struct qp_pool_s*                qp_pool_t;
typedef struct qp_manager_s*             qp_manager_t;
typedef struct qp_manager_elm_s*         qp_manager_elm_t;


qp_pool_t
qp_pool_init(qp_pool_t pool, size_t elmsize, size_t count);


qp_int_t
qp_pool_destroy(qp_pool_t pool, bool force);


void*
qp_pool_alloc(qp_pool_t pool, size_t size);


qp_int_t
qp_pool_free(qp_pool_t pool, void* ptr);


size_t
qp_pool_available(qp_pool_t pool);


size_t
qp_pool_used(qp_pool_t pool);


void*
qp_pool_to_array(qp_pool_t pool, size_t index);


qp_manager_t
qp_manager_init(qp_manager_t manager, size_t elmsize, size_t count);


qp_int_t
qp_manager_destroy(qp_manager_t manager, bool force);


size_t
qp_manager_used(qp_manager_t manager);


void*
qp_manager_alloc(qp_manager_t manager, size_t size);


qp_int_t
qp_manager_free(qp_manager_t manager, void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* QP_POOL_H */
