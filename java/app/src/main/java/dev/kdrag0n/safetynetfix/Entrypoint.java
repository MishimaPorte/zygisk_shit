package dev.kdrag0n.safetynetfix;

import android.os.Build;
import android.util.Log;

import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;

import dev.kdrag0n.safetynetfix.hooks.ClassLoaderHook;
import dev.kdrag0n.safetynetfix.hooks.PathClassLoaderHook;
import dev.kdrag0n.safetynetfix.hooks.RadioVersion;
import dev.kdrag0n.safetynetfix.hooks.TMHook;
import dev.kdrag0n.safetynetfix.hooks.TokenStore;

public class Entrypoint {
    public static void init() {
        Log.i("GMS-exploit-java", "start hooking");
        XposedHelpers.findAndHookMethod("android.os.Build", XposedBridge.BOOTCLASSLOADER, "getRadioVersion", new RadioVersion("kekstring"));

        try {
//            XposedHelpers.findAndHookMethod("java.lang.ClassLoader", XposedBridge.BOOTCLASSLOADER, "loadClass", "java/lang/String", new ClassLoaderHook());
//            XposedHelpers.findAndHookMethod("dalvik.system.DexFile", XposedBridge.BOOTCLASSLOADER, "loadClass", "java/lang/String","java/lang/ClassLoader", new ClassLoaderHook());
            XposedHelpers.findAndHookConstructor("dalvik.system.PathClassLoader", XposedBridge.BOOTCLASSLOADER, "java/lang/String","java/lang/ClassLoader", new PathClassLoaderHook());
            XposedHelpers.findAndHookConstructor("dalvik.system.PathClassLoader", XposedBridge.BOOTCLASSLOADER, "java/lang/String","java/lang/String","java/lang/ClassLoader", new PathClassLoaderHook());
        } catch (Throwable e) {
            Log.i("GMS-exploit-java", "unable to hook PathClassLoader", e);
        }
        Log.i("GMS-exploit-java", "end hooking");
    }
}
