package dev.kdrag0n.safetynetfix.hooks;

import android.util.Log;

import com.lody.whale.xposed.XC_MethodHook;
import com.lody.whale.xposed.XC_MethodReplacement;

public class RadioVersion extends XC_MethodReplacement {
    private String spoofedRadioVersion;
    public RadioVersion(String spoofedRadioVersion) {
        this.spoofedRadioVersion = spoofedRadioVersion;
    }
    @Override
    protected Object replaceHookedMethod(XC_MethodHook.MethodHookParam param) throws Throwable {
        Log.i("GMS-exploit-java", "getting spoofedRadioVersion");
        return spoofedRadioVersion;
    }
}
