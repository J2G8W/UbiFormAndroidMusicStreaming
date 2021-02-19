#include <jni.h>
#include <string>
#include <UbiForm/Component.h>
#include <fstream>

Component * component;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ubiformandroidstreamingexample_MainActivity_startComponent(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jstring ip_address) {
    try {
        if (component == nullptr) {
            jboolean isCopy = false;
            std::string componentUrl = env->GetStringUTFChars(ip_address, &isCopy);
            component = new Component(componentUrl);
        }
        if (component->getBackgroundPort() == -1) {
            component->startBackgroundListen();
        }
    } catch (std::logic_error &e) {
        std::string returnString = "Error with component startup: " + std::string(e.what());
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ubiformandroidstreamingexample_MainActivity_openFile(JNIEnv *env, jobject thiz,
                                                                      jstring file_loc) {
    jboolean isCopy = false;
    std::string fileLoc = env->GetStringUTFChars(file_loc, &isCopy);
    std::ifstream file;
    file.open(fileLoc);
    if (file.is_open()) {
        char bytes[10];
        file.read(bytes, 10);
        bytes[9] = 0;
        return env->NewStringUTF(bytes);
    }else{
        std::string ret = "Error " + fileLoc;
        return env->NewStringUTF(ret.c_str());
    }
}