package dev.kdrag0n.safetynetfix.hooks;

import android.util.Log;

import com.lody.whale.xposed.XC_MethodHook;

public class TokenStore extends XC_MethodHook {
    @Override
    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
        String token = (String) param.getResult();
        Log.i("GMS-exploit-java", "token is " + token);
    }
}
