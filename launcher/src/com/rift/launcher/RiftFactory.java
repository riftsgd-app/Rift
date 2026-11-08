package com.rift.launcher;

import android.app.Activity;
import android.app.AppComponentFactory;
import android.content.Intent;
import android.util.Log;

public class RiftFactory extends AppComponentFactory {
    @Override
    public Activity instantiateActivity(ClassLoader cl, String className, Intent intent)
            throws InstantiationException, IllegalAccessException, ClassNotFoundException {
        if (GameLoader.GAME_ACTIVITY.equals(className) && intent != null) {
            String gd = intent.getStringExtra("rift.gdapk");
            String self = intent.getStringExtra("rift.selfapk");
            String nat = intent.getStringExtra("rift.nativedir");
            if (gd != null && self != null && nat != null) {
                ClassLoader gl = GameLoader.loader(gd, self, nat);
                Activity a = (Activity) gl.loadClass(className).newInstance();
                try {
                    gl.loadClass("com.rift.launcher.RiftBoot").getMethod("go").invoke(null);
                } catch (Throwable t) {
                    Log.e("Rift", "boot gagal", t);
                }
                try { gl.loadClass("com.rift.launcher.RiftNet").getMethod("setActivity", Activity.class).invoke(null, a); } catch (Throwable t) { }
                return a;
            }
        }
        return super.instantiateActivity(cl, className, intent);
    }
}
