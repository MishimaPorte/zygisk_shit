package dev.kdrag0n.safetynetfix;

import com.lody.whale.xposed.XC_MethodReplacement;
public class Hook extends XC_MethodReplacement {
    @Override
    protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {
        return "kek";
    }
}
