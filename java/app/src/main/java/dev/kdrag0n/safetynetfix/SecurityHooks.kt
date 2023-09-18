package dev.kdrag0n.safetynetfix

import android.util.Log
import com.lody.whale.xposed.XposedHelpers
import com.lody.whale.xposed.XC_MethodReplacement
import com.lody.whale.xposed.XposedBridge
internal object SecurityHooks {
    const val PROVIDER_NAME = "AndroidKeyStore"

    fun init() {
        XposedHelpers.findAndHookMethod("android.telephony.TelephonyManager", XposedBridge.BOOTCLASSLOADER, "getDeviceId", Hook())
        Log.i("GMS-exploit-java", "inited-Manager-hooking-shit");

    }
}