#include "com_coapie_piebox_NativeDaemons.h"

#include "server/logger.h"
#include "server/sharepie.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <android/log.h>

static sharepie_t __sp;

static sharepie_t *get_spie(){
	return &__sp;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    setNativeRepoParam
 * Signature: (Ljava/lang/String;S)I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_setNativeRepoParam
  (JNIEnv *env, jobject obj, jstring rdir, jshort port){

	const char *dir = (*env)->GetStringUTFChars(env, rdir, 0);

	log_info("setNativeRepoParam :%s %d\n", dir,  port);

	spie_set_param(get_spie(), dir, port);

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

	return 0;
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

	spie_start(get_spie(), 1);

	return 0;
}

/*
 * Class:     com_coapie_piebox_NativeDaemons
 * Method:    stopNativeRepo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_coapie_piebox_NativeDaemons_stopNativeRepo
  (JNIEnv *env, jobject obj){
	log_info("stopNativeRepo\n");

	spie_stop(get_spie());
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
    if(spie_init(get_spie()) != 0){
        log_warn("init sharepie struct fail!\n");
        return -1;
    }

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		log_warn("JNI_OnLoad GetEnv failed!\n");
		return result;
	}

	log_info("JNI_OnLoad have success called!\n");

	result = JNI_VERSION_1_4;
	return result;
}
