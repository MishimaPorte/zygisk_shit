package dev.kdrag0n.safetynetfix.hooks;

import android.util.Log;

import com.lody.whale.xposed.XC_MethodHook;
import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;

public class PathClassLoaderHook extends XC_MethodHook{
    @Override
    protected void beforeHookedMethod(XC_MethodHook.MethodHookParam param) throws Throwable {
        Log.i("GMS-exploit-java", "loading path " + param.args[0] + " with classloader " + param.args[-1]);
//        switch ((String) param.args[0]) {
//            case "":
//                try {
//                    XposedHelpers.findAndHookMethod("android.content.ContextWrapper", XposedBridge.BOOTCLASSLOADER, "getSystemService", "java/lang/String", new TMHook());
//                } catch (Throwable e) {
//                    Log.i("GMS-exploit-java", "unable to hook Context Wrapper", e);
//                }
//                break;
//        }
    }
}