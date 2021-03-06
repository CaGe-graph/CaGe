/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class lisken_systoolbox_Pipe */

#ifndef _Included_lisken_systoolbox_Pipe
#define _Included_lisken_systoolbox_Pipe
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     lisken_systoolbox_Pipe
 * Method:    startPipe
 * Signature: ([Ljava/lang/Object;II[B[BZ[B)V
 */
JNIEXPORT void JNICALL Java_lisken_systoolbox_Pipe_startPipe
  (JNIEnv *, jobject, jobjectArray, jint, jint, jbyteArray, jbyteArray, jboolean, jbyteArray);

/*
 * Class:     lisken_systoolbox_Pipe
 * Method:    isRunning
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_lisken_systoolbox_Pipe_checkExitStatus
  (JNIEnv *, jobject);

/*
 * Class:     lisken_systoolbox_Pipe
 * Method:    waitForExit
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_lisken_systoolbox_Pipe_waitForExit
  (JNIEnv *, jobject);

/*
 * Class:     lisken_systoolbox_Pipe
 * Method:    stop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_lisken_systoolbox_Pipe_stop
  (JNIEnv *, jobject);

/*
 * Class:     lisken_systoolbox_Pipe
 * Method:    finalizePipe
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_lisken_systoolbox_Pipe_finalizePipe
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
