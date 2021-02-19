#include <jni.h>
#include <string>
#include <UbiForm/Component.h>
#include <fstream>


void writeToText(const std::string &textToWrite, JNIEnv *env,
                 jobject textObject) {
    jclass TextViewClass = env->FindClass(
            "com/example/ubiformandroidstreamingexample/MainActivity");
    jmethodID setText = env->GetMethodID(TextViewClass, "updateMainOutput",
                                         "(Ljava/lang/String;)V");
    jstring msg = env->NewStringUTF(textToWrite.c_str());
    env->CallVoidMethod(textObject, setText, msg);
}


struct PairStreamInfo{
    std::string currentFile;
    jobject activity_object;
    JNIEnv* env;
};
Component * component;
PairStreamInfo* pairStreamInfo;

void onStreamEnd(PairEndpoint *pEndpoint, void *pVoid){
    auto* file = static_cast<std::ifstream*>(pVoid);
    if(file->is_open()){
        file->close();
    }
    delete file;
}
void onPairStreamCreation(Endpoint * e, void* userData){
    // We use the global rather than the local data provided
    PairEndpoint* senderEndpoint = component->castToPair(e);
    auto* file = new std::ifstream ;
    file->open(pairStreamInfo->currentFile);
    if (file->is_open()) {
        SocketMessage sm;
        sm.addMember("extraInfo","HELLO");
        try {
            senderEndpoint->sendStream(*file, 10002, false, sm, onStreamEnd, file);
        } catch (std::logic_error &e){
            writeToText(e.what(),pairStreamInfo->env,pairStreamInfo->activity_object);
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ubiformandroidstreamingexample_MainActivity_startComponent(JNIEnv *env, jobject thiz, jstring ip_address,
                                                                            jobject activity_object) {
    try {
        if (component == nullptr) {
            jboolean isCopy = false;
            std::string componentUrl = env->GetStringUTFChars(ip_address, &isCopy);
            component = new Component(componentUrl);
            pairStreamInfo = new PairStreamInfo;
            pairStreamInfo->activity_object = activity_object;
            pairStreamInfo->env = env;

            std::shared_ptr<EndpointSchema> send = std::make_shared<EndpointSchema>();
            send->addProperty("extraInfo",ValueType::String);
            send->addRequired("extraInfo");
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            component->getComponentManifest().addEndpoint(SocketType::Pair,"sender",empty,send);


            component->registerStartupFunction("sender",onPairStreamCreation, nullptr);
        }
        if (component->getBackgroundPort() == -1) {
            component->startBackgroundListen();
        }
        writeToText("Started at: " + component->getSelfAddress(), env, activity_object);
    } catch (std::logic_error &e) {
        std::string returnString = "Error with component startup: " + std::string(e.what());
        writeToText(returnString, env, activity_object);
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ubiformandroidstreamingexample_MainActivity_openFile(JNIEnv *env, jobject thiz,
                                                                      jstring file_loc) {
    jboolean isCopy = false;
    pairStreamInfo->currentFile = env->GetStringUTFChars(file_loc, &isCopy);
    return env->NewStringUTF("Success");
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
        delete pairStreamInfo;
        pairStreamInfo = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ubiformandroidstreamingexample_MainActivity_addRDH(JNIEnv *env, jobject thiz,
                                                                    jstring url,
                                                                    jobject activity_object) {
    try {
        jboolean isCopy = false;
        std::string rdhUrl = env->GetStringUTFChars(url, &isCopy);
        component->getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhUrl);
        writeToText("Success adding Resource Discovery Hub", env, activity_object);
    } catch (std::logic_error &e) {
        writeToText(e.what(), env, activity_object);
    }
}