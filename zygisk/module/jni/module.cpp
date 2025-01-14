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

#define J(x) env->NewStringUTF(x)
#define CL(x) env->FindClass(x)

namespace safetynetfix {


// static jstring (*orig_logger_entry_max)(JNIEnv *env);
// static jstring my_logger_entry_max(JNIEnv *env) { 
//     // LOGI("device id is got i believe");
//     return orig_logger_entry_max(env);
// };
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
            injectPayload();
        }
    }

    void preServerSpecialize(zygisk::ServerSpecializeArgs *args) override {
        // Never tamper with system_server
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    zygisk::Api *api;
    JNIEnv *env;
    off_t size;

    std::vector<char> moduleDex;
    std::vector<char> nativeLibs;

    static int receiveFile(int remote_fd, std::vector<char>& buf) {
        off_t size;
        int ret = read(remote_fd, &size, sizeof(size));
        if (ret < 0) {
            LOGE("Failed to read size");
            return -1;
        }

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

        return bytesReceived;
    }

    void loadPayload() {
        auto fd = api->connectCompanion();

        auto size = receiveFile(fd, moduleDex);
        auto sizelibs = receiveFile(fd, nativeLibs);

        close(fd);
    }
    std::string getPublicStaticString(JNIEnv *env, jclass clazz, const char *fieldName, const char *val) {
        if (clazz != nullptr) {
            jfieldID fid = env->GetStaticFieldID(clazz, fieldName, "Ljava/lang/String;");
            if (fid != nullptr) {
                env->SetStaticObjectField(clazz, fid, env->NewStringUTF(val));
                jstring GladioReceiver = (jstring) env->GetStaticObjectField(clazz, fid);
                jboolean blnIsCopy;
                std::string mystr = env->GetStringUTFChars(GladioReceiver, &blnIsCopy);
                return mystr;
            }
        }
        return "ERROR";
    }

    void preSpecialize(const std::string& process) {
        jclass osbuild = CL("android/os/Build");
        // std::string device = getPublicStaticString(env, osbuild, "DEVICE", "goeatshit");
        std::string id = getPublicStaticString(env, osbuild, "ID", "goeatshit");
        // std::string serial = getPublicStaticString(env, osbuild, "SERIAL", "goeatshit");
        // std::string board = getPublicStaticString(env, osbuild, "BOARD", "goeatshit");
        // std::string brand = getPublicStaticString(env, osbuild, "BRAND", "goeatshit");
        std::string display = getPublicStaticString(env, osbuild, "DISPLAY", "goeatshit");
        std::string manufacturer = getPublicStaticString(env, osbuild, "MANUFACTURER", "goeatshit");
        std::string model = getPublicStaticString(env, osbuild, "MODEL", "goeatshit");
        std::string product = getPublicStaticString(env, osbuild, "PRODUCT", "goeatshit");
        std::string aid = getPublicStaticString(env, CL("android/provider/Settings$Secure"), "ANDROID_ID", "goeatshit");
        std::string buildfp = getPublicStaticString(env, osbuild, "FINGERPRINT", "goeatshit/goeatshit/goeatshit:6.0.1/IMXOSOP5801910311S/62:user/release-keys");
        // LOGI("aid: %s\n", aid.c_str());
        // LOGI("model: %s\n", model.c_str());
        // LOGI("board: %s\n", board.c_str());
        // LOGI("brand: %s\n", brand.c_str());
        // LOGI("display: %s\n", display.c_str());
       // LOGI("manuf: %s\n", manufacturer.c_str());
        // LOGI("product: %s\n", product.c_str());


        api->setOption(zygisk::FORCE_DENYLIST_UNMOUNT);

        if (strncmp(process.c_str(), "com.yy", 6) == 0) {
            // LOGI("hijacking %s", process.c_str());
            loadPayload();
            // LOGI("Payload loaded");
        }
    }

    void writeToJFile(jobject file, std::vector<char> data) {
        auto ff = CL("java/io/File");
        auto setWrit = env->GetMethodID(ff, "setWritable", "(Z)Z");
        auto canWrit = env->GetMethodID(ff, "canWrite", "()Z");
        auto can = env->CallBooleanMethod(file, canWrit);
        auto fos = CL("java/io/FileOutputStream");
        auto fosc = env->GetMethodID(fos, "<init>", "(Ljava/io/File;)V");
        auto fosi = env->NewObject(fos, fosc, file);
        jbyteArray jData = env->NewByteArray(data.size());
        env->SetByteArrayRegion(jData, 0, data.size(), (jbyte*)data.data());
        auto wrte = env->GetMethodID(fos, "write", "([B)V");
        env->CallObjectMethod(fosi, wrte, jData);
        auto clse = env->GetMethodID(fos, "close", "()V");
        env->CallObjectMethod(fosi, clse);
        env->DeleteLocalRef(jData);
    }

    void injectPayload() {
        auto getSystemClassLoader = env->GetStaticMethodID(CL("java/lang/ClassLoader"), "getSystemClassLoader","()Ljava/lang/ClassLoader;");
        auto systemClassLoader = env->CallStaticObjectMethod(CL("java/lang/ClassLoader"), getSystemClassLoader);


        auto ff = CL("java/io/File");
        auto newFile = env->GetMethodID(ff, "<init>", "(Ljava/lang/String;)V");
        auto doe = env->GetMethodID(ff, "deleteOnExit", "()V");
        auto getPath = env->GetMethodID(ff, "getAbsolutePath", "()Ljava/lang/String;");
        auto gettf = env->GetStaticMethodID(ff, "createTempFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/io/File;)Ljava/io/File;");

        auto libwhaleso = env->NewObject(ff, newFile, J("/data/data/com.yy.hagolite/files/libwhale.so"));
        auto cacheDirectFile = env->NewObject(ff, newFile, J("/data/data/com.yy.hagolite/files"));

        auto tempfileclasses = env->CallStaticObjectMethod(ff, gettf, J("classes"), J(".jar"), cacheDirectFile);
        env->CallVoidMethod(tempfileclasses, doe);
        env->CallVoidMethod(libwhaleso, doe);


        auto cacheDirP = env->CallObjectMethod(tempfileclasses, getPath);

        writeToJFile(tempfileclasses, moduleDex);
        writeToJFile(libwhaleso, nativeLibs);

        // tampering with DelegateLastClassLoader's DexPathList.
        // loading classes through DexFile because it has enumeration thing that is easy enough
        // It may be that i just fucked up all the classloading and Path one only is okay here, but i 
        // will deal with consequences later, if i will
        //
        // Initing this DexFile thing with our jar (apk) file
        auto dexCl = env->NewObject(CL("dalvik/system/DexFile"), env->GetMethodID(CL("dalvik/system/DexFile"), "<init>", "(Ljava/io/File;)V"), tempfileclasses);
        auto enumerDex = env->CallObjectMethod(dexCl, env->GetMethodID(CL("dalvik/system/DexFile"), "entries", "()Ljava/util/Enumeration;"));

        auto pathClInit = env->GetMethodID(CL("dalvik/system/PathClassLoader"), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
        auto pathClasses = env->NewObject(CL("dalvik/system/PathClassLoader"), pathClInit, cacheDirP, J("/data/data/com.yy.hagolite/files"), systemClassLoader);

        auto loadClass = env->GetMethodID(CL("dalvik/system/PathClassLoader"), "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
        
        //getting enumeration and iterating inside java 
        jobject entrypoint;
        auto en = CL("java/util/Enumeration");
        auto getnext = env->GetMethodID(en, "nextElement", "()Ljava/lang/Object;");
        auto hasmore = env->GetMethodID(en, "hasMoreElements", "()Z");
        for (;env->CallBooleanMethod(enumerDex, hasmore);) {
            auto entryClassName = env->CallObjectMethod(enumerDex, getnext);

            auto entryClassObj = env->CallObjectMethod(pathClasses, loadClass, entryClassName, JNI_TRUE);
            const char *name = env->GetStringUTFChars((jstring)entryClassName, 0);
            if (strcmp(name, "dev.kdrag0n.safetynetfix.Entrypoint") == 0) {
                // LOGI("loaded entrypoint %s", name);
                entrypoint = entryClassObj; 
            } else {
                env->DeleteLocalRef(entryClassObj);
            }

            env->DeleteLocalRef(entryClassName);
        }

        // LOGI("calling init");
        env->CallStaticVoidMethod((jclass)entrypoint, env->GetStaticMethodID((jclass)entrypoint, "init", "()V"));
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
    // LOGI("Sent module payload: %ld bytes", size);
    auto size2 = sendFile(remote_fd, LIB_PATH);
    // LOGI("Sent library payload: %ld bytes", size2);
}

}

REGISTER_ZYGISK_COMPANION(safetynetfix::companionHandler)
REGISTER_ZYGISK_MODULE(safetynetfix::SafetyNetFixModule)
