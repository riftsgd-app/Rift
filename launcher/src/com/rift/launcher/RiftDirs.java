package com.rift.launcher;

import android.content.Context;
import java.io.File;

// bikin folder Android/media/com.rift.launcher/{mods,logs,config} (bisa dibuka file manager)
public class RiftDirs {
    static void ensure(Context c) {
        File[] m = c.getExternalMediaDirs();
        if (m == null || m.length == 0 || m[0] == null) return;
        new File(m[0], "mods").mkdirs();
        new File(m[0], "logs").mkdirs();
        new File(m[0], "config").mkdirs();
    }
}
