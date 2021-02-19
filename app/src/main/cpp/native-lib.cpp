#include <jni.h>
#include <string>
#include <UbiForm/Component.h>
#include <fstream>



struct PairStreamInfo{
    Component* component;
    std::ifstream * file;
};
Component * component;
PairStreamInfo* pairStreamInfo;

void onPairStreamCreation(Endpoint * e, void* userData){
    // We use the global rather than the local data provided
    PairEndpoint* senderEndpoint = component->castToPair(e);
    SocketMessage sm;
    sm.addMember("extraInfo","HELLO");
    senderEndpoint->sendStream(*(pairStreamInfo->file), 10002, false, sm,nullptr, nullptr);
}

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


            std::shared_ptr<EndpointSchema> send = std::make_shared<EndpointSchema>();
            send->addProperty("extraInfo",ValueType::String);
            send->addRequired("extraInfo");
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            component->getComponentManifest().addEndpoint(SocketType::Pair,"sender",empty,send);

            pairStreamInfo = new PairStreamInfo;
            pairStreamInfo->component = component;
            pairStreamInfo->file = new std::ifstream;
            component->registerStartupFunction("sender",onPairStreamCreation, nullptr);
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
    pairStreamInfo->file->open(fileLoc);
    if (pairStreamInfo->file->is_open()) {
        return env->NewStringUTF("Success");
    }else{
        std::string ret = "Error " + fileLoc;
        return env->NewStringUTF(ret.c_str());
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ubiformandroidstreamingexample_MainActivity_deleteComponent(JNIEnv *env,
                                                                             jobject thiz) {
    if (component != nullptr){
        delete component;
        component = nullptr;
    }
    if (pairStreamInfo != nullptr){
        if (pairStreamInfo->file != nullptr){
            pairStreamInfo->file->close();
            delete pairStreamInfo->file;
        }
        delete pairStreamInfo;
    }
}