package dev.kdrag0n.safetynetfix.hooks;

import android.util.Log;

import com.lody.whale.xposed.XC_MethodHook;
import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;

public class ClassLoaderHook extends XC_MethodHook {
    @Override
    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
//        ClassLoader classLoader = (ClassLoader) param.thisObject;
//        Log.i("GMS-exploit-java", "loading class " + param.args[0] + " with classloader "+ param.thisObject.toString());
        try {
//            XposedHelpers.findClass("com.yy.pushsvc.simplify.TokenStore", classLoader);
//            Log.i("GMS-exploit-java", "hooked tokenstore");
        } catch (Throwable e) {
            Log.i("GMS-exploit-java", "unable to hook TokenStore", e);
        }
//        switch ((String) param.args[0]) {
//            case "":
//                break;
//        }
    }
}
