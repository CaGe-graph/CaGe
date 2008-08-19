
# include "jni_patches.h"

# include <jni.h>


/*
   An auxiliary function for native functions using CaGe.ElementRule:
   it calls the one important method of that class, but expects the
   relevant jclass and jmethodID values to be pre-computed and passed
   as arguments, as well as the jmethodID of java.lang.String's
   "getBytes" method (the one with no encoding specified).
*/

extern char *get_element
 (JNIEnv *env,
  jobject elementRule,
  jclass elementRuleClass, jmethodID getElement, jmethodID getBytes,
  jobject jGraph, jint i);


/*
   This function gets the Java classes and methods required by get_element.
   The first two arguments are input parameters, the following three are
   retrieved from the Java VM.
*/

extern void prepare_get_element
 (JNIEnv *env,
  jobject elementRule,
  jclass *elementRuleClass,
  jmethodID *getElement,
  jmethodID *getBytes);

