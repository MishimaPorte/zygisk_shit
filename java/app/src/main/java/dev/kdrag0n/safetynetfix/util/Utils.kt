package dev.kdrag0n.safetynetfix

import android.app.Application
import android.util.Log

private const val DEBUG = true
private const val TAG = "GMS_exploit-prototype"

internal fun <T> Any.get(name: String) = this::class.java.getDeclaredField(name).let { field ->
    field.isAccessible = true
    @Suppress("unchecked_cast")
    field.get(this) as T
}

internal fun logDebug(msg: String) {
    Log.i(TAG, "[${Application.getProcessName()}] $msg")
}

internal fun logDebug(msg: String, e: Throwable) {
    Log.i(TAG, "[${Application.getProcessName()}] $msg", e)
}
