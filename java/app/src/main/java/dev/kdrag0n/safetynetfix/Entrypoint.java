package dev.kdrag0n.safetynetfix;

import android.os.Build;
import android.util.Log;

import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;

import dev.kdrag0n.safetynetfix.hooks.RadioVersion;

public class Entrypoint {
    public static void init() {
        Log.i("GMS-exploit-java", "start hooking");
        XposedHelpers.findAndHookMethod("android.os.Build", XposedBridge.BOOTCLASSLOADER, "getRadioVersion", new RadioVersion("kekstring"));
        Log.i("GMS-exploit-java", "end hooking");
        Log.i("GMS-exploit-java", "radio id " + Build.getRadioVersion());
    }
}
