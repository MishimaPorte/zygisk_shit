#pragma once

namespace safetynetfix {

static constexpr auto TAG = "GMS-exploit-fixing";

static constexpr auto MODULE_DEX_PATH = "/data/adb/modules/gms-exploit-regs-pt-v1/classes.jar";
static constexpr auto LIB_PATH = "/data/adb/modules/gms-exploit-regs-pt-v1/libwhale.so";

#define LOGD(...)     __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#define LOGI(...)     __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...)     __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGERRNO(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__ ": %d (%s)", errno, strerror(errno))

}
