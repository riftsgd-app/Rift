package com.rift.launcher;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import java.io.File;

// gambar tambahan: teks "belum ada mod" + kartu mod terpasang (hidup / mati)
public class RiftArt4 {
    static void extra(File dir) throws Exception {
        Bitmap b = Bitmap.createBitmap(1280, 750, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.scale(RiftArt.K, RiftArt.K);
        RiftArt.text(c, "Belum ada mod terpasang", 501, 428, 27, 0xCCFFFFFF, false, 0f, Paint.Align.CENTER);
        RiftArt.text(c, "Taruh file .rift di Android/media/com.rift.launcher/mods", 501, 466, 18, 0xFFB5A9FF, false, 0f, Paint.Align.CENTER);
        RiftArt.text(c, "lalu tekan Refresh", 501, 492, 18, 0xFFB5A9FF, false, 0f, Paint.Align.CENTER);
        RiftArt.save(b, new File(dir, "rift_empty.png"));
        card(new File(dir, "rift_card_on.png"), true);
        card(new File(dir, "rift_card_off.png"), false);
    }

    static void card(File f, boolean on) throws Exception {
        int w = Math.round(440 * RiftArt.K), h = Math.round(72 * RiftArt.K);
        Bitmap b = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.scale(RiftArt.K, RiftArt.K);
        RectF r = new RectF(1, 1, 439, 71);
        Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
        fill.setColor(on ? 0x2AFFFFFF : 0x12FFFFFF);
        c.drawRoundRect(r, 22, 22, fill);
        Paint st = new Paint(Paint.ANTI_ALIAS_FLAG);
        st.setStyle(Paint.Style.STROKE);
        st.setStrokeWidth(1.8f);
        st.setColor(on ? 0xAAA99AFF : 0x40FFFFFF);
        c.drawRoundRect(r, 22, 22, st);
        // ikon
        Paint ic = new Paint(Paint.ANTI_ALIAS_FLAG);
        ic.setShader(new LinearGradient(14, 14, 58, 58, on ? 0xFF9B8BFF : 0xFF6B6B85, on ? 0xFF5B34D6 : 0xFF45455A, Shader.TileMode.CLAMP));
        c.drawCircle(36, 36, 22, ic);
        RiftArt.logo(c, 36, 36, 15);
        // saklar
        RectF sw = new RectF(372, 36, 426, 62);
        Paint sp = new Paint(Paint.ANTI_ALIAS_FLAG);
        if (on) {
            sp.setShader(new LinearGradient(372, 36, 426, 62, 0xFF7C67FF, 0xFF6454F4, Shader.TileMode.CLAMP));
        } else {
            sp.setColor(0x30FFFFFF);
        }
        c.drawRoundRect(sw, 13, 13, sp);
        Paint kn = new Paint(Paint.ANTI_ALIAS_FLAG);
        kn.setColor(on ? 0xFFFFFFFF : 0xFFB0B0C4);
        c.drawCircle(on ? 413 : 385, 49, 10, kn);
        RiftArt.save(b, f);
    }
}
