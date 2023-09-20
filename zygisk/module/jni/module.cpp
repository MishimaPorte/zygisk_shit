#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <android/log.h>
#include <iostream>
#include <fstream>

#include "zygisk.hpp"
#include "whale.h"
#include <string.h>
#include "module.h"

namespace safetynetfix {


static jstring (*orig_logger_entry_max)(JNIEnv *env);
static jstring my_logger_entry_max(JNIEnv *env) { 
    LOGI("device id is got i believe");
    return orig_logger_entry_max(env);
};
class SafetyNetFixModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char *rawProcess = env->GetStringUTFChars(args->nice_name, nullptr);
        if (rawProcess == nullptr) {
            return;
        }

        std::string process(rawProcess);
        env->ReleaseStringUTFChars(args->nice_name, rawProcess);

        preSpecialize(process);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        // Inject if module was loaded, otherwise this would've been unloaded by now (for non-GMS)
        if (!moduleDex.empty()) {
            LOGI("Injecting payload...");
            injectPayload();
            LOGI("Payload injected");
        }
    }

    void preServerSpecialize(zygisk::ServerSpecializeArgs *args) override {
        // Never tamper with system_server
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    zygisk::Api *api;
    JNIEnv *env;

    std::vector<char> moduleDex;

    static int receiveFile(int remote_fd, std::vector<char>& buf) {
        off_t size;
        int ret = read(remote_fd, &size, sizeof(size));
        if (ret < 0) {
            LOGE("Failed to read size");
            return -1;
        }

        std::ofstream myfile("/data/data/com.yy.hagolite/files/classes.jar", std::ios::out|std::ios::binary);
        
        buf.resize(size);

        int bytesReceived = 0;
        while (bytesReceived < size) {
            ret = read(remote_fd, buf.data() + bytesReceived, size - bytesReceived);
            if (ret < 0) {
                LOGE("Failed to read data");
                return -1;
            }

            bytesReceived += ret;
        }
        myfile.write(&buf[0], buf.size());
        myfile.close();

        return bytesReceived;
    }

    void loadPayload() {
        auto fd = api->connectCompanion();

        auto size = receiveFile(fd, moduleDex);
        LOGI("Loaded module payload: %d bytes", size);

        close(fd);
    }
    std::string getPublicStaticString(JNIEnv *env, const char *className, const char *fieldName) {
        jclass clazz = env->FindClass(className);
        if (clazz != nullptr) {
            jfieldID fid = env->GetStaticFieldID(clazz, fieldName, "Ljava/lang/String;");
            if (fid != nullptr) {
                // LOGI("go_eat_shit_start");
                env->SetStaticObjectField(clazz, fid, env->NewStringUTF("goeatshit"));
                // LOGI(".._end");
                jstring GladioReceiver = (jstring) env->GetStaticObjectField(clazz, fid);
                jboolean blnIsCopy;
                std::string mystr = env->GetStringUTFChars(GladioReceiver, &blnIsCopy);
                return mystr;
            }
        }
            return "ERROR";
    }

    void preSpecialize(const std::string& process) {
        LOGI("getting device");
        // std::string device = getPublicStaticString(env, "android/os/Build", "DEVICE");
        // std::string board = getPublicStaticString(env, "android/os/Build", "BOARD");
        // std::string brand = getPublicStaticString(env, "android/os/Build", "BRAND");
        // std::string display = getPublicStaticString(env, "android/os/Build", "DISPLAY");
        // std::string manufacturer = getPublicStaticString(env, "android/os/Build", "MANUFACTURER");
        // std::string model = getPublicStaticString(env, "android/os/Build", "MODEL");
        // std::string product = getPublicStaticString(env, "android/os/Build", "PRODUCT");
        // std::string aid = getPublicStaticString(env, "android/provider/Settings$Secure", "ANDROID_ID");
        // LOGI("aid: %s\n", aid.c_str());
        // LOGI("model: %s\n", model.c_str());
        // LOGI("board: %s\n", board.c_str());
        // LOGI("brand: %s\n", brand.c_str());
        // LOGI("display: %s\n", display.c_str());
       // LOGI("manuf: %s\n", manufacturer.c_str());
        // LOGI("product: %s\n", product.c_str());


        // Force DenyList unmounting for all GMS processes
        api->setOption(zygisk::FORCE_DENYLIST_UNMOUNT);

        // The unstable process is where SafetyNet attestation actually runs, so we only need to
        // spoof the model in that process. Leaving other processes alone fixes various issues
        // caused by model detection and flag provisioning, such as broken weather with the new
        // smartspace on Android 12.
        if (process == "com.yy.hagolite") {
            // Load the payload, but don't inject it yet until after specialization
            // Otherwise, specialization fails if any code from the payload still happens to be
            // running
            LOGI("Loading payload...");
            loadPayload();
            LOGI("Payload loaded");
        }
        // if (process == "com.google.android.gms.unstable") {
        //     // Load the payload, but don't inject it yet until after specialization
        //     // Otherwise, specialization fails if any code from the payload still happens to be
        //     // running
        //     LOGI("Loading payload...");
        //     loadPayload();
        //     LOGI("Payload loaded");
        // }
    }

    void injectPayload() {
        // First, get the system classloader
        auto ff = env->FindClass("java/io/File");
        auto clClass = env->FindClass("java/lang/ClassLoader");
        auto getSystemClassLoader = env->GetStaticMethodID(clClass, "getSystemClassLoader","()Ljava/lang/ClassLoader;");
        auto systemClassLoader = env->CallStaticObjectMethod(clClass, getSystemClassLoader);

        // Assuming we have a valid mapped module, load it. This is similar to the approach used for
        // Dynamite modules in GmsCompat, except we can use InMemoryDexClassLoader directly instead of

        // auto contxt = env->FindClass("android/content/Context");
        // auto getctx = env->GetStaticMethodID(contxt, "getApplicationContext","()Landroid/content/Context;");
        // LOGI("get system classloader");
        // auto ctx = env->CallStaticObjectMethod(contxt, getctx);
        // LOGI("get system classloader");
        // auto contxtgetCache = env->GetMethodID(contxt, "getCacheDir","()Ljava/io/File;");
        // LOGI("get system classloader");
        // auto cacheDir = env->CallObjectMethod(ctx, contxtgetCache);
        // LOGI("get system classloader");

        auto getPath = env->GetMethodID(ff, "getAbsolutePath", "()Ljava/lang/String;");
        auto cacheDirP = env->NewStringUTF("/data/data/com.yy.hagolite/files/classes.jar");
        auto libwhale = env->NewStringUTF("/data/data/com.yy.hagolite/files");
        auto cacheDir = env->NewStringUTF("/data/data/com.yy.hagolite/code_cache");

        auto newFile = env->GetMethodID(ff, "<init>", "(Ljava/lang/String;)V");
        auto mkdirs = env->GetMethodID(ff, "mkdirs", "()Z");
        auto classesDex = env->NewObject(ff, newFile, cacheDirP);
        auto cacheDirectFile = env->NewObject(ff, newFile, cacheDir);
        auto pth = env->CallObjectMethod(classesDex, getPath);
        auto cacheDirectStr = env->CallObjectMethod(cacheDirectFile, getPath);
        env->CallBooleanMethod(cacheDirectFile, mkdirs);

        

        // auto setWrit = env->GetMethodID(ff, "setWritable", "(Z)Z");
        // auto canWrit = env->GetMethodID(ff, "canWrite", "()Z");
        // auto can = env->CallBooleanMethod(classesDex, canWrit);
        // auto setted = env->CallBooleanMethod(cacheDirectFile, setWrit, JNI_TRUE);
        // auto fos = env->FindClass("java/io/FileOutputStream");
        // auto fosc = env->GetMethodID(fos, "<init>", "(Ljava/io/File;)V");
        // auto fosi = env->NewObject(fos, fosc, classesDex);
        // jbyteArray jData = env->NewByteArray(moduleDex.size());
        // env->SetByteArrayRegion(jData, 0, moduleDex.size(), (jbyte*)moduleDex.data());
        // auto wrte = env->GetMethodID(fos, "write", "([B)V");
        // env->CallObjectMethod(fosi, wrte, jData);
        // auto clse = env->GetMethodID(fos, "close", "()V");
        // env->CallObjectMethod(fosi, clse);
        // env->DeleteLocalRef(jData);

        // tampering with DelegateLastClassLoader's DexPathList.
        LOGI("create dex class loader");
        auto systemclass = env->FindClass("java/lang/System");
        auto enumerClass = env->FindClass("java/util/Enumeration");
        auto dexClClass = env->FindClass("dalvik/system/DexFile");
        auto dexClInit = env->GetMethodID(dexClClass, "<init>", "(Ljava/io/File;)V");
        auto dexCl = env->NewObject(dexClClass, dexClInit, classesDex);

        auto pathClassLoader = env->FindClass("dalvik/system/PathClassLoader");
        auto pathClInit = env->GetMethodID(pathClassLoader, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
        auto pathClasses = env->NewObject(pathClassLoader, pathClInit, cacheDirP, libwhale, systemClassLoader);
        // Load the class
        LOGI("load class");
        auto enumer = env->GetMethodID(dexClClass, "entries", "()Ljava/util/Enumeration;");
        auto hasnext = env->GetMethodID(enumerClass, "hasMoreElements", "()Z");
        auto nextEl = env->GetMethodID(enumerClass, "nextElement", "()Ljava/lang/Object;");
        auto enumerDex = env->CallObjectMethod(dexCl, enumer);

        auto loadClass = env->GetMethodID(pathClassLoader, "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
        jclass kek = (jclass) env->CallObjectMethod(pathClasses, loadClass, env->NewStringUTF("dev.kdrag0n.safetynetfix.Init"), systemClassLoader);
        if (env->ExceptionCheck()) { 
            LOGI("exception encountered");
            env->ExceptionDescribe();
        }
        for (;env->CallBooleanMethod(enumerDex, hasnext);) {
            auto entryClassName = env->CallObjectMethod(enumerDex, nextEl);
            const char *strReturn = env->GetStringUTFChars((jstring)entryClassName, 0);
            if (strcmp(strReturn, "kotlin") != 1) {
                if (strcmp(strReturn, "dev.kdrag0n.safetynetfix.Init") != 0) {
                    LOGI("load class %s", strReturn);
                    auto entryClassObj = env->CallObjectMethod(pathClasses, loadClass, entryClassName, JNI_TRUE);
                    env->DeleteLocalRef(std::exchange(entryClassObj, env->NewGlobalRef(entryClassObj)));
                } 
            }
            
            env->DeleteLocalRef(entryClassName);
        }

        LOGI("load class");

        // Call init. Static initializers don't run when merely calling loadClass from JNI.
        LOGI("call init");
        // auto entry = env->FindClass("dev/kdrag0n/safetynetfix/Init");
        // auto loadlib = env->GetStaticMethodID(systemclass, "load", "(Ljava/lang/String;)V");
        // env->CallStaticVoidMethod(systemclass, loadlib, env->NewStringUTF("/system/lib/libwhale.so"));
        // auto loadlib = env->GetStaticMethodID(systemclass, "load", "(Ljava/lang/String;)V");
        // env->CallStaticVoidMethod(systemclass, loadlib, env->NewStringUTF("/data/data/com.yy.hagolite/files/libwhale.so"));
        auto entryInit = env->GetStaticMethodID(kek, "init", "()V");
        env->CallStaticVoidMethod(kek, entryInit);
    }
};

static off_t sendFile(int remote_fd, const std::string& path) {
    auto in_fd = open(path.c_str(), O_RDONLY);
    if (in_fd < 0) {
        LOGE("Failed to open file %s: %d (%s)", path.c_str(), errno, strerror(errno));
        return -1;
    }

    auto size = lseek(in_fd, 0, SEEK_END);
    if (size < 0) {
        LOGERRNO("Failed to get file size");
        close(in_fd);
        return -1;
    }
    lseek(in_fd, 0, SEEK_SET);

    // Send size first for buffer allocation
    int ret = write(remote_fd, &size, sizeof(size));
    if (ret < 0) {
        LOGERRNO("Failed to send size");
        close(in_fd);
        return -1;
    }

    ret = sendfile(remote_fd, in_fd, nullptr, size);
    if (ret < 0) {
        LOGERRNO("Failed to send data");
        close(in_fd);
        return -1;
    }

    close(in_fd);
    return size;
}

static void companionHandler(int remote_fd) {
    // Serve module dex
    auto size = sendFile(remote_fd, MODULE_DEX_PATH);
    LOGI("Sent module payload: %ld bytes", size);
}

}

REGISTER_ZYGISK_COMPANION(safetynetfix::companionHandler)
REGISTER_ZYGISK_MODULE(safetynetfix::SafetyNetFixModule)
