
# include "get_element.h"

# include "jbytes.h"


void
prepare_get_element
 (JNIEnv *env,
  jobject elementRule,
  jclass *elementRuleClass,
  jmethodID *getElement,
  jmethodID *getBytes)
{
  jclass stringClass;
  stringClass = (*env)->FindClass (env, "java/lang/String");
  *getBytes = (*env)->GetMethodID (env, stringClass, "getBytes", "()[B");
  *elementRuleClass = (*env)->GetObjectClass (env, elementRule);
  *getElement = (*env)->GetMethodID (env, *elementRuleClass,
   "getElement", "(Lcage/EmbeddableGraph;I)Ljava/lang/String;");
}


char *get_element
 (JNIEnv *env,
  jobject elementRule,
  jclass elementRuleClass, jmethodID getElement, jmethodID getBytes,
  jobject jGraph, jint i)
{
  jobject elementString;
  jbyteArray elementBytes;
  elementString = (*env)->CallNonvirtualObjectMethod
   (env, elementRule, elementRuleClass, getElement, jGraph, i);
  if (elementString == NULL) return NULL;
  elementBytes = (*env)->CallObjectMethod (env, elementString, getBytes);
  return jbytes2string (env, elementBytes);
}

