package dev.kdrag0n.safetynetfix.hooks;


import android.util.Log;

import com.lody.whale.xposed.XC_MethodHook;

public class TMHook extends XC_MethodHook {
    @Override
    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
        if ((String) param.args[0] == "phone") {
//            param.setResult(null);
            Log.i("GMS-exploit-java", "Context service " + (String) param.args[0] + " returning null");
        }
    }


    @Override
    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
        Log.i("GMS-exploit-java", "Context service " + (String) param.args[0] + " returning " + param.getResult());
    }
}
