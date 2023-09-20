package dev.kdrag0n.safetynetfix;

import android.os.Build;
import android.util.Log;

import com.lody.whale.xposed.XC_MethodHook;
import com.lody.whale.xposed.XC_MethodReplacement;
import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;

public class Init extends XC_MethodReplacement {
    public static void init() {
        Log.i("GMS-exploit-java", "start hooking");
        XposedHelpers.findAndHookMethod("android.os.Build", XposedBridge.BOOTCLASSLOADER, "getRadioVersion", new Init());
        Log.i("GMS-exploit-java", "end hooking");
        Log.i("GMS-exploit-java", "radio id " + new Build().getRadioVersion());
    }
    @Override
    protected Object replaceHookedMethod(XC_MethodHook.MethodHookParam param) throws Throwable {
        Log.i("GMS-exploit-java", "getting did");
        return "kekstringid";
    }
}