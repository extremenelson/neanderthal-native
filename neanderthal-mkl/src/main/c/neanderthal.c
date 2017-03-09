//   Copyright (c) Dragan Djuric. All rights reserved.
//   The use and distribution terms for this software are covered by the
//   Eclipse Public License 1.0 (http://opensource.org/licenses/eclipse-1.0.php) or later
//   which can be found in the file LICENSE at the root of this distribution.
//   By using this software in any fashion, you are agreeing to be bound by
//   the terms of this license.
//   You must not remove this notice, or any other, from this software.

#include <stdlib.h>
#include <jni.h>
#include "mkl.h"
#include "uncomplicate_neanderthal_internal_host_CBLAS.h"
#include "uncomplicate_neanderthal_internal_host_MKL.h"
#include "uncomplicate_neanderthal_internal_host_LAPACK.h"


JavaVM *javavm;

// Note: this custom xerbla does not get called by MKL. I don't know why,
// but MKL keeps calling its default xerbla, which only prints the message to
// the output. Hopefully, Neanderthal will not send broken data, so this
// should not be an issue, but I'll have to investigate what's the source of
// this.
void xerbla(const char *srname, const int *info, const int lsrname) {

    JNIEnv *env;
    (*javavm)->AttachCurrentThread(javavm, (void **)&env, NULL);

    jclass iaexception = (*env)->FindClass(env, "java/lang/RuntimeException");
    (*env)->ThrowNew(env, iaexception, srname);
    return;

}

JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM *jvm, void *reserved) {
    javavm=jvm;
    mkl_set_xerbla(xerbla);
    return JNI_VERSION_1_2;
}

/*
 * ======================================================
 * Level 1 BLAS functions
 * ======================================================
 */

/*
 * ------------------------------------------------------
 * DOT
 * ------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sdsdot
(JNIEnv *env, jclass clazz, jint N, jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    return cblas_sdsdot(N, alpha, cX + offsetX, incX, cY + offsetY, incY);
};


JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsdot
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    return cblas_dsdot(N, cX + offsetX, incX, cY + offsetY, incY);
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ddot
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    return cblas_ddot(N, cX + offsetX, incX, cY + offsetY, incY);
};

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sdot
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    return cblas_sdot(N, cX + offsetX, incX, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * NRM2
 * ------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_snrm2
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    return cblas_snrm2(N, cX + offsetX, incX);
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dnrm2
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    return cblas_dnrm2(N, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * ASUM
 * ------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sasum
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    return cblas_sasum(N, cX + offsetX, incX);
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dasum
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    return cblas_dasum(N, cX + offsetX, incX);
};


/*
 * ------------------------------------------------------
 * BLAS PLUS: SUM
 * ------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssum
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X) + offsetX;

    const int stride = incX;
    const int step = 16;
    const int tail = N % step;
    const int n = N - tail;

    float *end = cX + n * incX;

    float acc = 0.0f;
    float acc0 = 0.0f;
    float acc1 = 0.0f;
    float acc2 = 0.0f;
    float acc3 = 0.0f;
    float acc4 = 0.0f;
    float acc5 = 0.0f;
    float acc6 = 0.0f;
    float acc7 = 0.0f;
    float acc8 = 0.0f;
    float acc9 = 0.0f;
    float acc10 = 0.0f;
    float acc11 = 0.0f;
    float acc12 = 0.0f;
    float acc13 = 0.0f;
    float acc14 = 0.0f;
    float acc15 = 0.0f;

    while (cX != end) {
        acc0 += cX[0];
        acc1 += cX[stride];
        acc2 += cX[2*stride];
        acc3 += cX[3*stride];
        acc4 += cX[4*stride];
        acc5 += cX[5*stride];
        acc6 += cX[6*stride];
        acc7 += cX[7*stride];
        acc8 += cX[8*stride];
        acc9 += cX[9*stride];
        acc10 += cX[10*stride];
        acc11 += cX[11*stride];
        acc12 += cX[12*stride];
        acc13 += cX[13*stride];
        acc14 += cX[14*stride];
        acc15 += cX[15*stride];
        cX += 16 * stride;
    }

    for (int i = 0; i < tail; i++) {
        acc += end[i * stride];
    }

    return acc + acc0 + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7
        + acc8 + acc9 + acc10 + acc11 + acc12 + acc13 + acc14 + acc15;
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsum
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X) + offsetX;

    const int stride = incX;
    const int step = 16;
    const int tail = N % step;
    const int n = N - tail;

    double *end = cX + n * incX;

    double acc = 0.0f;
    double acc0 = 0.0f;
    double acc1 = 0.0f;
    double acc2 = 0.0f;
    double acc3 = 0.0f;
    double acc4 = 0.0f;
    double acc5 = 0.0f;
    double acc6 = 0.0f;
    double acc7 = 0.0f;
    double acc8 = 0.0f;
    double acc9 = 0.0f;
    double acc10 = 0.0f;
    double acc11 = 0.0f;
    double acc12 = 0.0f;
    double acc13 = 0.0f;
    double acc14 = 0.0f;
    double acc15 = 0.0f;

    while (cX != end) {
        acc0 += cX[0];
        acc1 += cX[stride];
        acc2 += cX[2*stride];
        acc3 += cX[3*stride];
        acc4 += cX[4*stride];
        acc5 += cX[5*stride];
        acc6 += cX[6*stride];
        acc7 += cX[7*stride];
        acc8 += cX[8*stride];
        acc9 += cX[9*stride];
        acc10 += cX[10*stride];
        acc11 += cX[11*stride];
        acc12 += cX[12*stride];
        acc13 += cX[13*stride];
        acc14 += cX[14*stride];
        acc15 += cX[15*stride];
        cX += 16 * stride;
    }

    for (int i = 0; i < tail; i++) {
        acc += end[i * stride];
    }

    return acc + acc0 + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7
        + acc8 + acc9 + acc10 + acc11 + acc12 + acc13 + acc14 + acc15;
};


/*
 * ------------------------------------------------------
 * IAMAX
 * ------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_isamax
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    return (int)cblas_isamax(N, cX + offsetX, incX);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_idamax
(JNIEnv *env, jclass clazz, jint N, jobject X, jint offsetX, jint incX) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    return (int)cblas_idamax(N, cX + offsetX, incX);
};

/*
 * ======================================================
 * Level 1 BLAS procedures
 * ======================================================
 */

/*
 * ------------------------------------------------------
 * ROT
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_srot
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 float c, float s) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_srot(N, cX + offsetX, incX, cY + offsetY, incY, c, s);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_drot
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 double c, double s) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_drot(N, cX + offsetX, incX, cY + offsetY, incY, c, s);
};

/*
 * ------------------------------------------------------
 * ROTG
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_srotg
(JNIEnv *env, jclass clazz, jobject abcs, jint offset_abcs, jint inc_abcs) {

    float *c_abcs = (float *) (*env)->GetDirectBufferAddress(env, abcs) + offset_abcs;
    cblas_srotg(c_abcs, c_abcs + inc_abcs, c_abcs + 2 * inc_abcs, c_abcs + 3 * inc_abcs);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_drotg
(JNIEnv *env, jclass clazz, jobject abcs, jint offset_abcs, jint inc_abcs) {

    double *c_abcs = (double *) (*env)->GetDirectBufferAddress(env, abcs) + offset_abcs;
    cblas_drotg(c_abcs, c_abcs + inc_abcs, c_abcs + 2 * inc_abcs, c_abcs + 3 * inc_abcs);
};

/*
 * ------------------------------------------------------
 * ROTM
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_srotm
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject param) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    float *c_param = (float *) (*env)->GetDirectBufferAddress(env, param);
    cblas_srotm(N, cX + offsetX, incX, cY + offsetY, incY, c_param);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_drotm
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject param) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    double *c_param = (double *) (*env)->GetDirectBufferAddress(env, param);
    cblas_drotm(N, cX + offsetX, incX, cY + offsetY, incY, c_param);
};

/*
 * ------------------------------------------------------
 * ROTMG
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_srotmg
(JNIEnv *env, jclass clazz,
 jobject d1d2xy, jint offset_d1d2xy, jint inc_d1d2xy,
 jobject param) {

    float *c_d1d2xy = (float *) (*env)->GetDirectBufferAddress(env, d1d2xy) + offset_d1d2xy;
    float *c_param = (float *) (*env)->GetDirectBufferAddress(env, param);
    cblas_srotmg(c_d1d2xy, c_d1d2xy + inc_d1d2xy, c_d1d2xy + 2 * inc_d1d2xy, c_d1d2xy[3*inc_d1d2xy], c_param);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_drotmg
(JNIEnv *env, jclass clazz, jobject d1d2xy, jint offset_d1d2xy, jint inc_d1d2xy,
 jobject param) {

    double *c_d1d2xy = (double *) (*env)->GetDirectBufferAddress(env, d1d2xy) + offset_d1d2xy;
    double *c_param = (double *) (*env)->GetDirectBufferAddress(env, param);
    cblas_drotmg(c_d1d2xy, c_d1d2xy + inc_d1d2xy, c_d1d2xy + 2 * inc_d1d2xy, c_d1d2xy[3*inc_d1d2xy], c_param);
};

/*
 * ------------------------------------------------------
 * SWAP
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sswap
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_sswap(N, cX + offsetX, incX, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dswap
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dswap(N, cX + offsetX, incX, cY + offsetY, incY);

};

/*
 * ------------------------------------------------------
 * SCAL
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sscal
(JNIEnv *env, jclass clazz,
 jint N, jfloat alpha,
 jobject X, jint offsetX, jint incX) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_sscal(N, alpha, cX + offsetX, incX);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dscal
(JNIEnv *env, jclass clazz,
 jint N, jdouble alpha,
 jobject X, jint offsetX, jint incX) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dscal(N, alpha, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * COPY
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_scopy
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_scopy(N, cX + offsetX, incX, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dcopy
(JNIEnv *env, jclass clazz, jint N,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dcopy(N, cX + offsetX, incX, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * AXPY
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_saxpy
(JNIEnv *env, jclass clazz,
 jint N, jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_saxpy(N, alpha, cX + offsetX, incX, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_daxpy
(JNIEnv *env, jclass clazz,
 jint N, jdouble alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_daxpy(N, alpha, cX + offsetX, incX, cY + offsetY, incY);
};

/*
 * ======================================================
 * Level 2 BLAS procedures
 * ======================================================
 */

/*
 * ------------------------------------------------------
 * GEMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sgemv
(JNIEnv * env, jclass clazz,
 jint Order, jint TransA,
 jint M, jint N,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jfloat beta,
 jobject Y, jint offsetY, jint incY) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_sgemv(Order, TransA, M, N, alpha, cA + offsetA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dgemv
(JNIEnv * env, jclass clazz,
 jint Order, jint TransA,
 jint M, jint N,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jdouble beta,
 jobject Y, jint offsetY, jint incY) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dgemv(Order, TransA, M, N, alpha, cA + offsetA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * GBMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sgbmv
(JNIEnv *env, jclass clazz,
 jint Order, jint TransA,
 jint M, jint N,
 jint KL, jint KU,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jfloat beta,
 jobject Y, jint offsetY, jint incY) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_sgbmv(Order, TransA, M, N, KL, KU, alpha, cA + offsetA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dgbmv
(JNIEnv *env, jclass clazz,
 jint Order, jint TransA,
 jint M, jint N,
 jint KL, jint KU,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jdouble beta,
 jobject Y, jint offsetY, jint incY) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dgbmv(Order, TransA, M, N, KL, KU, alpha, cA + offsetA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * SYMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssymv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jfloat beta,
 jobject Y, jint offsetY, jint incY) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_ssymv(Order, Uplo, N, alpha, cA + offsetA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsymv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jdouble beta,
 jobject Y, jint offsetY, jint incY) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dsymv(Order, Uplo, N, alpha, cA + offsetA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * SBMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssbmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N, jint K,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jfloat beta,
 jobject Y, jint offsetY, jint incY) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_ssbmv(Order, Uplo, N, K, alpha, cA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsbmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N, jint K,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX,
 jdouble beta,
 jobject Y, jint offsetY, jint incY) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dsbmv(Order, Uplo, N, K, alpha, cA, lda,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * SPMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sspmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jfloat alpha,
 jobject Ap,
 jobject X, jint offsetX, jint incX,
 jfloat beta,
 jobject Y, jint offsetY, jint incY) {

    float *cAp = (float *) (*env)->GetDirectBufferAddress(env, Ap);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_sspmv(Order, Uplo, N, alpha, cAp,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dspmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jdouble alpha,
 jobject Ap,
 jobject X, jint offsetX, jint incX,
 jdouble beta,
 jobject Y, jint offsetY, jint incY) {

    double *cAp = (double *) (*env)->GetDirectBufferAddress(env, Ap);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_dspmv(Order, Uplo, N, alpha, cAp,
                cX + offsetX, incX, beta, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * TRMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_strmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_strmv(Order, Uplo, TransA, Diag, N, cA + offsetA, lda, cX + offsetX, incX);
};


JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtrmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dtrmv(Order, Uplo, TransA, Diag, N, cA + offsetA, lda, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * TBMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_stbmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N, jint K,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_strmv(Order, Uplo, TransA, Diag, N, cA + offsetA, lda, cX + offsetX, incX);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtbmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N, jint K,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dtrmv(Order, Uplo, TransA, Diag, N, cA + offsetA, lda, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * TPMV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_stpmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject Ap,
 jobject X, jint offsetX, jint incX) {

    float *cAp = (float *) (*env)->GetDirectBufferAddress(env, Ap);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_stpmv(Order, Uplo, TransA, Diag, N, cAp, cX + offsetX, incX);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtpmv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject Ap,
 jobject X, jint offsetX, jint incX) {

    double *cAp = (double *) (*env)->GetDirectBufferAddress(env, Ap);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dtpmv(Order, Uplo, TransA, Diag, N, cAp, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * TRSV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_strsv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_strsv(Order, Uplo, TransA, Diag, N, cA + offsetA, lda, cX + offsetX, incX);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtrsv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dtrsv(Order, Uplo, TransA, Diag, N, cA + offsetA, lda, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * TBSV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_stbsv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N, jint K,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_stbsv(Order, Uplo, TransA, Diag, N, K, cA + offsetA, lda, cX + offsetX, incX);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtbsv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N, jint K,
 jobject A, jint offsetA, jint lda,
 jobject X, jint offsetX, jint incX) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dtbsv(Order, Uplo, TransA, Diag, N, K, cA + offsetA, lda, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * TPSV
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_stpsv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject Ap,
 jobject X, jint offsetX, jint incX) {

    float *cAp = (float *) (*env)->GetDirectBufferAddress(env, Ap);
    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    cblas_stpsv(Order, Uplo, TransA, Diag, N, cAp, cX + offsetX, incX);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtpsv
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint TransA, jint Diag,
 jint N,
 jobject Ap,
 jobject X, jint offsetX, jint incX) {

    double *cAp = (double *) (*env)->GetDirectBufferAddress(env, Ap);
    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    cblas_dtpsv(Order, Uplo, TransA, Diag, N, cAp, cX + offsetX, incX);
};

/*
 * ------------------------------------------------------
 * GER
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sger
(JNIEnv *env, jclass clazz,
 jint Order,
 jint M, jint N,
 jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject A, jint offsetA, jint lda) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    cblas_sger(Order, M, N, alpha, cX + offsetX, incX, cY + offsetY, incY, cA + offsetA, lda);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dger
(JNIEnv *env, jclass clazz,
 jint Order,
 jint M, jint N,
 jdouble alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject A, jint offsetA, jint lda) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    cblas_dger(Order, M, N, alpha, cX + offsetX, incX, cY + offsetY, incY, cA + offsetA, lda);
};

/*
 * ------------------------------------------------------
 * SYR
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssyr
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject A, jint offsetA, jint lda) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    cblas_ssyr(Order, Uplo, N, alpha, cX + offsetX, incX, cA + offsetA, lda);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsyr
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jdouble alpha,
 jobject X, jint offsetX, jint incX,
 jobject A, jint offsetA, jint lda) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    cblas_dsyr(Order, Uplo, N, alpha, cX + offsetX, incX, cA + offsetA, lda);
}

/*
 * ------------------------------------------------------
 * SPR
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sspr
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject Ap) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cAp = (float *) (*env)->GetDirectBufferAddress(env, Ap);
    cblas_sspr(Order, Uplo, N, alpha, cX + offsetX, incX, cAp);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dspr
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jdouble alpha,
 jobject X, jint offsetX, jint incX,
 jobject Ap) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cAp = (double *) (*env)->GetDirectBufferAddress(env, Ap);
    cblas_dspr(Order, Uplo, N, alpha, cX + offsetX, incX, cAp);
};

/*
 * ------------------------------------------------------
 * SYR2
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssyr2
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject A, jint offsetA, jint lda) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    cblas_ssyr2(Order, Uplo, N, alpha, cX + offsetX, incX, cY + offsetY, incY, cA + offsetA, lda);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsyr2
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jdouble alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject A, jint offsetA, jint lda) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    cblas_dsyr2(Order, Uplo, N, alpha, cX + offsetX, incX, cY + offsetY, incY, cA + offsetA, lda);
};

/*
 * ------------------------------------------------------
 * SPR2
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sspr2
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jfloat alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject Ap) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    float *cAp = (float *) (*env)->GetDirectBufferAddress(env, Ap);
    cblas_sspr2(Order, Uplo, N, alpha, cX + offsetX, incX, cY + offsetY, incY, cAp);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dspr2
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo,
 jint N,
 jdouble alpha,
 jobject X, jint offsetX, jint incX,
 jobject Y, jint offsetY, jint incY,
 jobject Ap) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    double *cAp = (double *) (*env)->GetDirectBufferAddress(env, Ap);
    cblas_dspr2(Order, Uplo, N, alpha, cX + offsetX, incX, cY + offsetY, incY, cAp);
};

/*
 * ======================================================
 * Level 3 BLAS procedures
 * ======================================================
 */


/*
 * ------------------------------------------------------
 * GEMM
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_sgemm
(JNIEnv *env, jclass clazz,
 jint Order, jint TransA, jint TransB,
 jint M, jint N, jint K,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb,
 jfloat beta,
 jobject C, jint offsetC, jint ldc) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    float *cC = (float *) (*env)->GetDirectBufferAddress(env, C);
    cblas_sgemm(Order, TransA, TransB, M, N, K, alpha,
                cA + offsetA, lda, cB + offsetB, ldb, beta, cC + offsetC, ldc);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dgemm
(JNIEnv *env, jclass clazz,
 jint Order, jint TransA, jint TransB,
 jint M, jint N, jint K,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb,
 jdouble beta,
 jobject C, jint offsetC, jint ldc) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    double *cC = (double *) (*env)->GetDirectBufferAddress(env, C);
    cblas_dgemm(Order, TransA, TransB, M, N, K, alpha,
                cA + offsetA, lda, cB + offsetB, ldb, beta, cC + offsetC, ldc);
};

/*
 * ------------------------------------------------------
 * SYMM
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssymm
(JNIEnv *env, jclass clazz,
 jint Order, jint Side, jint Uplo,
 jint M, jint N,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb,
 jfloat beta,
 jobject C, jint offsetC, jint ldc) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    float *cC = (float *) (*env)->GetDirectBufferAddress(env, C);
    cblas_ssymm(Order, Side, Uplo, M, N, alpha,
                cA + offsetA, lda, cB + offsetB, ldb, beta, cC + offsetC, ldc);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsymm
(JNIEnv *env, jclass clazz,
 jint Order, jint Side, jint Uplo,
 jint M, jint N,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb,
 jdouble beta,
 jobject C, jint offsetC, jint ldc) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    double *cC = (double *) (*env)->GetDirectBufferAddress(env, C);
    cblas_dsymm(Order, Side, Uplo, M, N, alpha,
                cA + offsetA, lda, cB + offsetB, ldb, beta, cC + offsetC, ldc);
};

/*
 * ------------------------------------------------------
 * SYRK
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssyrk
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint Trans,
 jint N, jint K,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jfloat beta,
 jobject C, jint offsetC, jint ldc) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cC = (float *) (*env)->GetDirectBufferAddress(env, C);
    cblas_ssyrk(Order, Uplo, Trans, N, K, alpha, cA + offsetA, lda, beta, cC + offsetC, ldc);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsyrk
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint Trans,
 jint N, jint K,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jfloat beta,
 jobject C, jint offsetC, jint ldc) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cC = (double *) (*env)->GetDirectBufferAddress(env, C);
    cblas_dsyrk(Order, Uplo, Trans, N, K, alpha, cA + offsetA, lda, beta, cC + offsetC, ldc);
};

/*
 * ------------------------------------------------------
 * SYR2K
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_ssyr2k
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint Trans,
 jint N, jint K,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb,
 jfloat beta,
 jobject C, jint offsetC, jint ldc) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    float *cC = (float *) (*env)->GetDirectBufferAddress(env, C);
    cblas_ssyr2k(Order, Uplo, Trans, N, K, alpha,
                 cA + offsetA, lda, cB + offsetB, ldb, beta, cC + offsetC, ldc);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dsyr2k
(JNIEnv *env, jclass clazz,
 jint Order, jint Uplo, jint Trans,
 jint N, jint K,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb,
 jdouble beta,
 jobject C, jint offsetC, jint ldc) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    double *cC = (double *) (*env)->GetDirectBufferAddress(env, C);
    cblas_dsyr2k(Order, Uplo, Trans, N, K, alpha,
                 cA + offsetA, lda, cB + offsetB, ldb, beta, cC + offsetC, ldc);
};

/*
 * ------------------------------------------------------
 * TRMM
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_strmm
(JNIEnv *env, jclass clazz,
 jint Order, jint Side,
 jint Uplo, jint TransA, jint Diag,
 jint M, jint N,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    cblas_strmm(Order, Side, Uplo, TransA, Diag, M, N, alpha,
                cA + offsetA, lda, cB + offsetB, ldb);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtrmm
(JNIEnv *env, jclass clazz,
 jint Order, jint Side,
 jint Uplo, jint TransA, jint Diag,
 jint M, jint N,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    cblas_dtrmm(Order, Side, Uplo, TransA, Diag, M, N, alpha,
                cA + offsetA, lda, cB + offsetB, ldb);
};

/*
 * ------------------------------------------------------
 * TRSM
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_strsm
(JNIEnv *env, jclass clazz,
 jint Order, jint Side,
 jint Uplo, jint TransA, jint Diag,
 jint M, jint N,
 jfloat alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    cblas_strsm(Order, Side, Uplo, TransA, Diag, M, N, alpha,
                cA + offsetA, lda, cB + offsetB, ldb);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_CBLAS_dtrsm
(JNIEnv *env, jclass clazz,
 jint Order, jint Side,
 jint Uplo, jint TransA, jint Diag,
 jint M, jint N,
 jdouble alpha,
 jobject A, jint offsetA, jint lda,
 jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    cblas_dtrsm(Order, Side, Uplo, TransA, Diag, M, N, alpha,
                cA + offsetA, lda, cB + offsetB, ldb);
};


/*
 * ======================================================
 * MKL functions
 * ======================================================
 */

/*
* ------------------------------------------------------
* AXPBY
* ------------------------------------------------------
*/

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_saxpby
(JNIEnv *env, jclass clazz,
 jint N, jfloat alpha, jobject X, jint offsetX, jint incX,
 jfloat beta, jobject Y, jint offsetY, jint incY) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    float *cY = (float *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_saxpby(N, alpha, cX + offsetX, incX, beta, cY + offsetY, incY);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_daxpby
(JNIEnv *env, jclass clazz,
 jint N, jdouble alpha, jobject X, jint offsetX, jint incX,
 jdouble beta, jobject Y, jint offsetY, jint incY) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    double *cY = (double *) (*env)->GetDirectBufferAddress(env, Y);
    cblas_daxpby(N, alpha, cX + offsetX, incX, beta, cY + offsetY, incY);
};

/*
 * ------------------------------------------------------
 * MATADD
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_somatadd
(JNIEnv *env, jclass clazz, jint ordering, jint transa, jint transb, jint M, jint N,
 jfloat alpha, jobject A, jint offsetA, jint lda, jfloat beta, jobject B, jint offsetB, jint ldb,
 jobject C, jint offsetC, jint ldc) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    float *cC = (float *) (*env)->GetDirectBufferAddress(env, C);
    mkl_somatadd((char)ordering, (char)transa, (char)transb, (size_t)M, (size_t)N,
                 alpha, cA + offsetA, (size_t)lda, beta, cB + offsetB, (size_t)ldb,
                 cC + offsetC, (size_t)ldc);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_domatadd
(JNIEnv *env, jclass clazz, jint ordering, jint transa, jint transb, jint M, jint N,
 jdouble alpha, jobject A, jint offsetA, jint lda, jdouble beta, jobject B, jint offsetB, jint ldb,
 jobject C, jint offsetC, jint ldc) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    double *cC = (double *) (*env)->GetDirectBufferAddress(env, C);
    mkl_domatadd((char)ordering, (char)transa, (char)transb, (size_t)M, (size_t)N,
                 alpha, cA + offsetA, (size_t)lda, beta, cB + offsetB, (size_t)ldb,
                 cC + offsetC, (size_t)ldc);
};

/*
 * ------------------------------------------------------
 * SOMATCOPY
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_somatcopy
(JNIEnv *env, jclass clazz, jint ordering, jint trans, jint M, jint N,
 jfloat alpha, jobject A, jint offsetA, jint lda, jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    mkl_somatcopy((char)ordering, (char)trans, (size_t)M, (size_t)N,
                  alpha, cA + offsetA, (size_t)lda, cB + offsetB, (size_t)ldb);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_domatcopy
(JNIEnv *env, jclass clazz, jint ordering, jint trans, jint M, jint N,
 jdouble alpha, jobject A, jint offsetA, jint lda, jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    mkl_domatcopy((char)ordering, (char)trans, (size_t)M, (size_t)N,
                  alpha, cA + offsetA, (size_t)lda, cB + offsetB, (size_t)ldb);
};

/*
 * ------------------------------------------------------
 * SIMATCOPY
 * ------------------------------------------------------
 */

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_simatcopy
(JNIEnv *env, jclass clazz, jint ordering, jint trans, jint M, jint N,
 jfloat alpha, jobject AB, jint offsetAB, jint lda, jint ldb) {

    float *cAB = (float *) (*env)->GetDirectBufferAddress(env, AB);
    mkl_simatcopy((char)ordering, (char)trans, (size_t)M, (size_t)N,
                  alpha, cAB + offsetAB, (size_t)lda, (size_t)ldb);
};

JNIEXPORT void JNICALL Java_uncomplicate_neanderthal_internal_host_MKL_dimatcopy
(JNIEnv *env, jclass clazz, jint ordering, jint trans, jint M, jint N,
 jdouble alpha, jobject AB, jint offsetAB, jint lda, jint ldb) {

    double *cAB = (double *) (*env)->GetDirectBufferAddress(env, AB);
    mkl_dimatcopy((char)ordering, (char)trans, (size_t)M, (size_t)N,
                  alpha, cAB + offsetAB, (size_t)lda, (size_t)ldb);
};


/*
 * ===========================================================
 *                  LAPACK
 * ===========================================================
 */

/*
 * -----------------------------------------------------------------
 * Auxiliary Routines
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------
 * LANGE
 * -----------------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slange
(JNIEnv *env, jclass clazz, jint Order, jint norm, jint M, jint N, jobject A, jint offsetA, jint lda) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_slange(Order, (char)norm, M, N, cA + offsetA, lda);
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlange
(JNIEnv *env, jclass clazz, jint Order, jint norm, jint M, jint N, jobject A, jint offsetA, jint lda) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_dlange(Order, (char)norm, M, N, cA + offsetA, lda);
};

/*
 * -----------------------------------------------------------------
 * LANSY
 * -----------------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slansy
(JNIEnv *env, jclass clazz, jint Order, jint norm, jint uplo, jint N, jobject A, jint offsetA, jint lda) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_slansy(Order, (char)norm, (char)uplo, N, cA + offsetA, lda);
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlansy
(JNIEnv *env, jclass clazz, jint Order, jint norm, jint uplo, jint N, jobject A, jint offsetA, jint lda) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_dlansy(Order, (char)norm, (char)uplo, N, cA + offsetA, lda);
};

/*
 * -----------------------------------------------------------------
 * LANTR
 * -----------------------------------------------------------------
 */

JNIEXPORT jfloat JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slantr
(JNIEnv *env, jclass clazz, jint Order, jint norm, jint uplo, jint diag, jint M, jint N,
 jobject A, jint offsetA, jint lda) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_slantr(Order, (char)norm, (char)uplo, (char)diag, M, N, cA + offsetA, lda);
};

JNIEXPORT jdouble JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlantr
(JNIEnv *env, jclass clazz, jint Order, jint norm, jint uplo, jint diag, jint M, jint N,
 jobject A, jint offsetA, jint lda) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_dlantr(Order, (char)norm, (char)uplo, (char)diag, M, N, cA + offsetA, lda);
};

/*
 * -----------------------------------------------------------------
 * LACPY
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slacpy
(JNIEnv *env, jclass clazz, jint Order, jint uplo, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    return LAPACKE_slacpy(Order, (char)uplo, M, N, cA + offsetA, lda, cB + offsetB, ldb);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlacpy
(JNIEnv *env, jclass clazz, jint Order, jint uplo, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    return LAPACKE_dlacpy(Order, (char)uplo, M, N, cA + offsetA, lda, cB + offsetB, ldb);
};

/*
 * -----------------------------------------------------------------
 * LASET
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slaset
(JNIEnv *env, jclass clazz, jint Order, jint uplo, jint M, jint N,
 jfloat alpha, jfloat beta, jobject A, jint offsetA, jint lda) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_slaset(Order, (char)uplo, M, N, alpha, beta, cA + offsetA, lda);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlaset
(JNIEnv *env, jclass clazz, jint Order, jint uplo, jint M, jint N,
 jdouble alpha, jdouble beta, jobject A, jint offsetA, jint lda) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_dlaset(Order, (char)uplo, M, N, alpha, beta, cA + offsetA, lda);
};

/*
 * -----------------------------------------------------------------
 * LASCL
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slascl
(JNIEnv *env, jclass clazz, jint Order, jint type, jint k1, jint ku, jfloat cfrom, jfloat cto,
 jint M, jint N, jobject A, jint offsetA, jint lda) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_slascl(Order, (char)type, k1, ku, cfrom, cto, M, N, cA + offsetA, lda);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlascl
(JNIEnv *env, jclass clazz, jint Order, jint type, jint k1, jint ku, jdouble cfrom, jdouble cto,
 jint M, jint N, jobject A, jint offsetA, jint lda) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    return LAPACKE_dlascl(Order, (char)type, k1, ku, cfrom, cto, M, N, cA + offsetA, lda);
};

/*
 * -----------------------------------------------------------------
 * LASWP
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slaswp
(JNIEnv *env, jclass clazz, jint Order, jint N, jobject A, jint offsetA, jint lda,
 jint k1, jint k2, jobject ipiv, jint offsetIpiv, jint incIpiv) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    int *cIpiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_slaswp(Order, N, cA + offsetA, lda, k1, k2, cIpiv + offsetIpiv, incIpiv);
};


JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlaswp
(JNIEnv *env, jclass clazz, jint Order, jint N, jobject A, jint offsetA, jint lda,
 jint k1, jint k2, jobject ipiv, jint offsetIpiv, jint incIpiv) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    int *cIpiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_dlaswp(Order, N, cA + offsetA, lda, k1, k2, cIpiv + offsetIpiv, incIpiv);
};

/*
 * -----------------------------------------------------------------
 * LASRT
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_slasrt
(JNIEnv *env, jclass clazz, jint id, jint N, jobject X, jint offsetX) {

    float *cX = (float *) (*env)->GetDirectBufferAddress(env, X);
    return LAPACKE_slasrt((char)id, N, cX + offsetX);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dlasrt
(JNIEnv *env, jclass clazz, jint id, jint N, jobject X, jint offsetX) {

    double *cX = (double *) (*env)->GetDirectBufferAddress(env, X);
    return LAPACKE_dlasrt((char)id, N, cX + offsetX);
};

/*
 * -----------------------------------------------------------------
 * Linear Equation Routines
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------
 * GETRF
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgetrf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject ipiv, jint offsetIpiv) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    int *cipiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_sgetrf(Order, M, N, cA + offsetA, lda, cipiv + offsetIpiv);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgetrf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject ipiv, jint offsetIpiv) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    int *cipiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_dgetrf(Order, M, N, cA + offsetA, lda, cipiv + offsetIpiv);
};

/*
 * -----------------------------------------------------------------
 * GETRS
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgetrs
(JNIEnv *env, jclass clazz, jint Order, jint trans, jint N, jint nrhs,
 jobject A, jint offsetA, jint lda, jobject ipiv, jint offsetIpiv, jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    int *cipiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_sgetrs(Order, (char)trans, N, nrhs, cA + offsetA, lda,
                          cipiv + offsetIpiv, cB + offsetB, ldb);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgetrs
(JNIEnv *env, jclass clazz, jint Order, jint trans, jint N, jint nrhs,
 jobject A, jint offsetA, jint lda, jobject ipiv, jint offsetIpiv, jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    int *cipiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_dgetrs(Order, (char)trans, N, nrhs, cA + offsetA, lda,
                          cipiv + offsetIpiv, cB + offsetB, ldb);
};

/*
 * -----------------------------------------------------------------
 * GESV
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgesv
(JNIEnv *env, jclass clazz, jint Order, jint N, jint nrhs, jobject A, jint offsetA, jint lda,
 jobject ipiv, jint offsetIpiv, jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    int *cipiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_sgesv(Order, N, nrhs, cA + offsetA, lda, cipiv + offsetIpiv, cB + offsetB, ldb);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgesv
(JNIEnv *env, jclass clazz, jint Order, jint N, jint nrhs, jobject A, jint offsetA, jint lda,
 jobject ipiv, jint offsetIpiv, jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    int *cipiv = (int *) (*env)->GetDirectBufferAddress(env, ipiv);
    return LAPACKE_dgesv(Order, N, nrhs, cA + offsetA, lda, cipiv + offsetIpiv, cB + offsetB, ldb);
};

/*
 * -----------------------------------------------------------------
 * Orthogonal Factorization Routines
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------
 * GEQRF
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgeqrf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cTau = (float *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_sgeqrf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgeqrf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cTau = (double *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_dgeqrf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

/*
 * -----------------------------------------------------------------
 * GEQRFP
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgeqrfp
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cTau = (float *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_sgeqrfp(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgeqrfp
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cTau = (double *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_dgeqrfp(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};


/*
 * -----------------------------------------------------------------
 * GERQF
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgerqf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cTau = (float *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_sgerqf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgerqf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cTau = (double *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_dgerqf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

/*
* -----------------------------------------------------------------
* GELQF
* -----------------------------------------------------------------
*/

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgelqf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cTau = (float *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_sgelqf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgelqf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cTau = (double *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_dgelqf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

/*
* -----------------------------------------------------------------
* GELQF
* -----------------------------------------------------------------
*/

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgeqlf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cTau = (float *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_sgeqlf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgeqlf
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject tau, jint offsetTau) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cTau = (double *) (*env)->GetDirectBufferAddress(env, tau);
    return LAPACKE_dgeqlf(Order, M, N, cA + offsetA, lda, cTau + offsetTau);
};

/*
 * -----------------------------------------------------------------
 * Linear Least Squares Routines
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------
 * GELS
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgels
(JNIEnv *env, jclass clazz, jint Order, jint trans, jint M, jint N, jint nrhs,
 jobject A, jint offsetA, jint lda, jobject B, jint offsetB, jint ldb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cB = (float *) (*env)->GetDirectBufferAddress(env, B);
    return LAPACKE_sgels(Order, (char)trans, M, N, nrhs, cA + offsetA, lda, cB + offsetB, ldb);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgels
(JNIEnv *env, jclass clazz, jint Order, jint trans, jint M, jint N, jint nrhs,
 jobject A, jint offsetA, jint lda, jobject B, jint offsetB, jint ldb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cB = (double *) (*env)->GetDirectBufferAddress(env, B);
    return LAPACKE_dgels(Order, (char)trans, M, N, nrhs, cA + offsetA, lda, cB + offsetB, ldb);
};

/*
 * -----------------------------------------------------------------
 * Non-Symmetric Eigenvalue Problem Routines
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgeev
(JNIEnv *env, jclass clazz, jint Order, jint jobvr, jint jobvl, jint N,
 jobject A, jint offsetA, jint lda, jobject WR, jint offsetWR, jobject WL, jint offsetWL,
 jobject VL, jint offsetVL, jint ldvl, jobject VR, jint offsetVR, jint ldvr) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cWR = (float *) (*env)->GetDirectBufferAddress(env, WR);
    float *cWL = (float *) (*env)->GetDirectBufferAddress(env, WL);
    float *cVR = (float *) (*env)->GetDirectBufferAddress(env, VR);
    float *cVL = (float *) (*env)->GetDirectBufferAddress(env, VL);
    return LAPACKE_sgeev(Order, (char)jobvr, (char)jobvl, N, cA + offsetA, lda,
                         cWR + offsetWR, cWL + offsetWL, cVL + offsetVL, ldvl, cVR + offsetVR, ldvl);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgeev
(JNIEnv *env, jclass clazz, jint Order, jint jobvr, jint jobvl, jint N,
 jobject A, jint offsetA, jint lda, jobject WR, jint offsetWR, jobject WL, jint offsetWL,
 jobject VL, jint offsetVL, jint ldvl, jobject VR, jint offsetVR, jint ldvr) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cWR = (double *) (*env)->GetDirectBufferAddress(env, WR);
    double *cWL = (double *) (*env)->GetDirectBufferAddress(env, WL);
    double *cVR = (double *) (*env)->GetDirectBufferAddress(env, VR);
    double *cVL = (double *) (*env)->GetDirectBufferAddress(env, VL);
    return LAPACKE_dgeev(Order, (char)jobvr, (char)jobvl, N, cA + offsetA, lda,
                         cWR + offsetWR, cWL + offsetWL, cVL + offsetVL, ldvl, cVR + offsetVR, ldvl);
};

/*
 * -----------------------------------------------------------------
 * Singular Value Decomposition Routines
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------
 * GEBRD
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgebrd
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject D, jint offsetD, jobject E, jint offsetE,
 jobject tauq, jint offsetTauq, jobject taup, jint offsetTaup) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cD = (float *) (*env)->GetDirectBufferAddress(env, D);
    float *cE = (float *) (*env)->GetDirectBufferAddress(env, E);
    float *cTauq = (float *) (*env)->GetDirectBufferAddress(env, tauq);
    float *cTaup = (float *) (*env)->GetDirectBufferAddress(env, taup);
    return LAPACKE_sgebrd(Order, M, N, cA + offsetA, lda, cD + offsetD, cE + offsetE,
                          cTauq + offsetTauq, cTaup + offsetTaup);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgebrd
(JNIEnv *env, jclass clazz, jint Order, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject D, jint offsetD, jobject E, jint offsetE,
 jobject tauq, jint offsetTauq, jobject taup, jint offsetTaup) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cD = (double *) (*env)->GetDirectBufferAddress(env, D);
    double *cE = (double *) (*env)->GetDirectBufferAddress(env, E);
    double *cTauq = (double *) (*env)->GetDirectBufferAddress(env, tauq);
    double *cTaup = (double *) (*env)->GetDirectBufferAddress(env, taup);
    return LAPACKE_dgebrd(Order, M, N, cA + offsetA, lda, cD + offsetD, cE + offsetE,
                          cTauq + offsetTauq, cTaup + offsetTaup);
};

/*
 * -----------------------------------------------------------------
 * BDSQR
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sbdsqr
(JNIEnv *env, jclass clazz, jint Order, jint uplo, jint N, jint ncvt, jint nru, jint ncc,
 jobject D, jint offsetD, jobject E, jint offsetE, jobject VT, jint offsetVT, jint ldvt,
 jobject U, jint offsetU, jint ldu, jobject C, jint offsetC, jint ldc) {

    float *cD = (float *) (*env)->GetDirectBufferAddress(env, D);
    float *cE = (float *) (*env)->GetDirectBufferAddress(env, E);
    float *cVT = (float *) (*env)->GetDirectBufferAddress(env, VT);
    float *cU = (float *) (*env)->GetDirectBufferAddress(env, U);
    float *cC = (float *) (*env)->GetDirectBufferAddress(env, C);
    return LAPACKE_sbdsqr(Order, (char)uplo, N, ncvt, nru, ncc, cD + offsetD, cE + offsetE,
                          cVT + offsetVT, ldvt, cU + offsetU, ldu, cC + offsetC, ldc);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dbdsqr
(JNIEnv *env, jclass clazz, jint Order, jint uplo, jint N, jint ncvt, jint nru, jint ncc,
 jobject D, jint offsetD, jobject E, jint offsetE, jobject VT, jint offsetVT, jint ldvt,
 jobject U, jint offsetU, jint ldu, jobject C, jint offsetC, jint ldc) {

    double *cD = (double *) (*env)->GetDirectBufferAddress(env, D);
    double *cE = (double *) (*env)->GetDirectBufferAddress(env, E);
    double *cVT = (double *) (*env)->GetDirectBufferAddress(env, VT);
    double *cU = (double *) (*env)->GetDirectBufferAddress(env, U);
    double *cC = (double *) (*env)->GetDirectBufferAddress(env, C);
    return LAPACKE_dbdsqr(Order, (char)uplo, N, ncvt, nru, ncc, cD + offsetD, cE + offsetE,
                          cVT + offsetVT, ldvt, cU + offsetU, ldu, cC + offsetC, ldc);
};

/*
 * -----------------------------------------------------------------
 * GESVD
 * -----------------------------------------------------------------
 */

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_sgesvd
(JNIEnv *env, jclass clazz, jint Order, jint jobu, jint jobvt, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject S, jint offsetS, jobject U, jint offsetU, jint ldu,
 jobject VT, jint offsetVT, jint ldvt, jobject superb, jint offsetSuperb) {

    float *cA = (float *) (*env)->GetDirectBufferAddress(env, A);
    float *cS = (float *) (*env)->GetDirectBufferAddress(env, S);
    float *cU = (float *) (*env)->GetDirectBufferAddress(env, U);
    float *cVT = (float *) (*env)->GetDirectBufferAddress(env, VT);
    float *cSuperb = (float *) (*env)->GetDirectBufferAddress(env, superb);
    return LAPACKE_sgesvd(Order, (char)jobu, (char)jobvt, M, N, cA + offsetA, lda, cS + offsetS,
                          cU + offsetU, ldu, cVT + offsetVT, ldvt, cSuperb + offsetSuperb);
};

JNIEXPORT jint JNICALL Java_uncomplicate_neanderthal_internal_host_LAPACK_dgesvd
(JNIEnv *env, jclass clazz, jint Order, jint jobu, jint jobvt, jint M, jint N,
 jobject A, jint offsetA, jint lda, jobject S, jint offsetS, jobject U, jint offsetU, jint ldu,
 jobject VT, jint offsetVT, jint ldvt, jobject superb, jint offsetSuperb) {

    double *cA = (double *) (*env)->GetDirectBufferAddress(env, A);
    double *cS = (double *) (*env)->GetDirectBufferAddress(env, S);
    double *cU = (double *) (*env)->GetDirectBufferAddress(env, U);
    double *cVT = (double *) (*env)->GetDirectBufferAddress(env, VT);
    double *cSuperb = (double *) (*env)->GetDirectBufferAddress(env, superb);
    return LAPACKE_dgesvd(Order, (char)jobu, (char)jobvt, M, N, cA + offsetA, lda, cS + offsetS,
                          cU + offsetU, ldu, cVT + offsetVT, ldvt, cSuperb + offsetSuperb);
};