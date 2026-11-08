package com.rift.launcher;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.text.InputType;
import android.view.WindowManager;
import android.widget.EditText;
import java.io.File;

// dialog Android di atas game: kolom search dan konfirmasi hapus mod
final class RiftUi {
    static volatile Activity game;
    static volatile String result = "";

    static String poll() {
        String r = result;
        result = "";
        return r;
    }

    static String ui(final String cmd) {
        final Activity a = game;
        if (a == null) return "";
        a.runOnUiThread(new Runnable() {
            public void run() {
                try { show(a, cmd); } catch (Throwable t) { }
            }
        });
        return "OK";
    }

    static void show(final Activity a, String cmd) {
        if (cmd.startsWith("search:")) {
            final EditText et = new EditText(a);
            et.setText(cmd.substring(7));
            et.setSingleLine(true);
            et.setHint("Cari mod...");
            et.setInputType(InputType.TYPE_CLASS_TEXT);
            et.setSelectAllOnFocus(true);
            AlertDialog d = new AlertDialog.Builder(a, android.R.style.Theme_Material_Dialog_Alert)
                    .setTitle("Search mods")
                    .setView(et)
                    .setPositiveButton("Cari", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface x, int w) { result = "=" + et.getText().toString().trim(); }
                    })
                    .setNeutralButton("Hapus filter", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface x, int w) { result = "="; }
                    })
                    .setNegativeButton("Batal", null)
                    .create();
            d.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
            d.show();
        } else if (cmd.startsWith("del:")) {
            final String base = cmd.substring(4);
            new AlertDialog.Builder(a, android.R.style.Theme_Material_Dialog_Alert)
                    .setTitle("Hapus mod?")
                    .setMessage(base + ".rift akan dihapus. Berlaku setelah game di-restart.")
                    .setPositiveButton("Hapus", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface x, int w) {
                            if (remove(a, base)) result = "!";
                        }
                    })
                    .setNegativeButton("Batal", null)
                    .show();
        }
    }

    static boolean remove(Activity a, String base) {
        boolean ok = false;
        try {
            File[] m = a.getExternalMediaDirs();
            if (m != null && m.length > 0 && m[0] != null) {
                File f = new File(m[0], "mods/" + base + ".rift");
                ok = f.delete();
            }
            File[] kids = new File(a.getFilesDir(), "modcache").listFiles();
            if (kids != null) {
                for (File k : kids) {
                    if (k.getName().matches(java.util.regex.Pattern.quote(base) + "-[0-9]+-[0-9]+")) rm(k);
                }
            }
        } catch (Throwable t) { }
        return ok;
    }

    static void rm(File f) {
        File[] k = f.listFiles();
        if (k != null) for (File c : k) rm(c);
        f.delete();
    }
}
