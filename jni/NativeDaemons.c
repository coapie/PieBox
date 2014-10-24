#include "com_coapie_piebox_NativeDaemons.h"

#include "server/logger.h"
#include "server/server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <android/log.h>


static server_t __server;
static char __server_rdir[1024] = "";
static short __server_port = 8086;

static struct sockaddr_in __sin;
static struct stat __rs;

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    setNativeRepoParam
 * Signature: (Ljava/lang/String;S)I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_setNativeRepoParam
  (JNIEnv *env, jobject obj, jstring rdir, jshort port){
	char *dir = (*env)->GetStringUTFChars(env, rdir, 0);

	log_info("setNativeRepoParam :%s %d\n", dir,  port);

	__server_port = port;
	strcpy(__server_rdir, dir);

	(*env)->ReleaseStringUTFChars(env, rdir, dir);


	return 0;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    getNativeRepoStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_getNativeRepoStatus
  (JNIEnv *env, jobject obj){
	log_info("getNativeRepoStatus\n");

	return __server.status;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    startNativeRepo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_startNativeRepo
  (JNIEnv *env, jobject obj){
	int rc;

	log_info("startNativeRepo\n");

	 rc = stat(__server_rdir, &__rs);
	 if(rc != 0){
		 log_warn("startNativeRepo fail:rdir is not exists\n");
		 return -1;
	 }

    __sin.sin_family = AF_INET;
    __sin.sin_addr.s_addr = 0;
    __sin.sin_port = htons(__server_port);

    return server_main(&__server, (struct sockaddr *)&__sin, sizeof(__sin), __server_rdir);
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    stopNativeRepo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_stopNativeRepo
  (JNIEnv *env, jobject obj){
	log_info("stopNativeRepo\n");

	return 0;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    restartNativeRepo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_restartNativeRepo
  (JNIEnv *env, jobject obj){
	log_info("restartNativeRepo\n");

	Java_com_coapie_piebox_NativeDaemons_stopNativeRepo(env, obj);
	Java_com_coapie_piebox_NativeDaemons_startNativeRepo(env, obj);

	return 0;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    cleanNativeRepo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_cleanNativeRepo
  (JNIEnv *env, jobject obj){
    log_info("clean native repo\n");

    return 0;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    setNativeTransParam
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_setNativeTransParam
  (JNIEnv *env, jobject obj, jstring rdir){
	return -1;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    getNativeTransStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_getNativeTransStatus
  (JNIEnv *env, jobject obj){
	return -1;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    startNativeTrans
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_startNativeTrans
  (JNIEnv *env, jobject obj){
	return -1;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    stopNativeTrans
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_stopNativeTrans
  (JNIEnv *env, jobject obj){
	return -1;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    restartNativeTrans
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_restartNativeTrans
  (JNIEnv *env, jobject obj){
	return -1;
}

/*
* Set some test stuff up.
*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env;
	jint result = -1;

	logger_init();

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		log_warn("JNI_OnLoad GetEnv failed!\n");
		return result;
	}

	log_info("JNI_OnLoad have success called!\n");

	result = JNI_VERSION_1_4;
	return result;
}
