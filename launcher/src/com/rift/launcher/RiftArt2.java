package com.rift.launcher;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import java.io.File;

// latar ungu + bola cahaya untuk menu Rift
public class RiftArt2 {
    static void extra(File dir) throws Exception {
        Bitmap d = Bitmap.createBitmap(64, 64, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(d);
        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        p.setShader(new RadialGradient(32, 32, 46, 0xE63A1C8A, 0xF0120833, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, 64, 64, p);
        RiftArt.save(d, new File(dir, "rift_dim.png"));
        orb(new File(dir, "rift_orb1.png"), 0x8B5CFF);
        orb(new File(dir, "rift_orb2.png"), 0xC27BFF);
        orb(new File(dir, "rift_orb3.png"), 0x5B6CFF);
    }

    static void orb(File f, int rgb) throws Exception {
        Bitmap b = Bitmap.createBitmap(256, 256, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        p.setShader(new RadialGradient(128, 128, 128, 0xB0000000 | rgb, rgb, Shader.TileMode.CLAMP));
        c.drawCircle(128, 128, 128, p);
        RiftArt.save(b, f);
    }
}
