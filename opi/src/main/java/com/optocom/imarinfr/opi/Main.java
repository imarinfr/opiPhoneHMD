package com.optocom.imarinfr.opi;

import android.app.AlertDialog;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.widget.PopupMenu;
import androidx.appcompat.app.AppCompatActivity;

public class Main extends AppCompatActivity implements PopupMenu.OnMenuItemClickListener {
    static {
        System.loadLibrary("cardboard_jni");
    }

    private static long nativeApp;
    private OpiConnection opiConnection;
    private GLSurfaceView glView;

    private SensorListener sensorListener;

    @Override
    protected void onCreate(Bundle savedInstance) {
        super.onCreate(savedInstance);
        // get the connection to native methods
        nativeApp = nativeOnCreate();
        setContentView(R.layout.activity_vr);
        glView = findViewById(R.id.surface_view);
        // configuration
        glView.setEGLConfigChooser(8, 8, 8, 8, 16, 8);
        glView.setEGLContextClientVersion(3);
        // Set up connections to OPI R, renderer, and controller
        Renderer renderer = new Renderer(nativeApp, glView);
        glView.setRenderer(renderer);
        glView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        glView.setFocusable(true); // TODO: see if we can get the volume up and down
        glView.setOnTouchListener(this::onTouch);
        //glView.setOnKeyListener(this);  // TODO: see if we can get the volume up and down
        glView.setOnClickListener(v->renderer.onTriggerEvent());
        //TODO: performance issues?
        setImmersiveSticky();
        View decorView = getWindow().getDecorView();
        decorView.setOnSystemUiVisibilityChangeListener(
                (visibility) -> {
                    if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                        setImmersiveSticky();
                    }
                });
        // set layout brightness to maximum
        WindowManager.LayoutParams layout = getWindow().getAttributes();
        layout.screenBrightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_FULL;
        getWindow().setAttributes(layout);
        // Prevents screen from dimming/locking.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // create a light sensor listener
        sensorListener = new SensorListener(glView.getContext());
        // get OPI connection ready
        opiConnection = new OpiConnection(glView.getContext(), nativeApp, renderer, sensorListener);
    }

    @Override
    protected void onPause() {
        super.onPause();
        nativeOnPause(nativeApp);
        glView.onPause();
        sensorListener.getSensorManager().unregisterListener(sensorListener);
    }

    @Override
    protected void onResume() {
        super.onResume();
        glView.onResume();
        nativeOnResume(nativeApp);
        sensorListener.getSensorManager().registerListener(sensorListener,
                sensorListener.getSensor(), SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        nativeOnDestroy(nativeApp);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            setImmersiveSticky();
        }
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        if (item.getItemId() == R.id.switch_viewer) {
            nativeOnSwitchViewer(nativeApp);
            return true;
        }
        if (item.getItemId() == R.id.show_connection) {
            ShowInternetConnection();
            return true;
        }
        return false;
    }

    // Callback for when a key is touched
    public boolean onTouch(View v, MotionEvent event) {
        if(event.getAction() == MotionEvent.ACTION_DOWN) {
            v.performClick();
            return true;
        } else return false;
    }

    // Callback for when close button is pressed
    public void closeOpiApp(View view) {
        finish();
    }

    // Callback for when settings_menu button is pressed
    public void showSettings(View view) {
        PopupMenu popup = new PopupMenu(this, view);
        MenuInflater inflater = popup.getMenuInflater();
        inflater.inflate(R.menu.settings_menu, popup.getMenu());
        popup.setOnMenuItemClickListener(this);
        popup.show();
    }

    private void ShowInternetConnection() {
        AlertDialog.Builder builder = new AlertDialog.Builder(Main.this);
        if (opiConnection.socketAddress() != null) {
            builder.setTitle("Connection details");
            builder.setMessage("Server socket is " + opiConnection.socketAddress());
        } else {
            builder.setTitle("Network Problem");
            builder.setMessage("There's no network detected. Please check your network connection.");
        }
        builder.setPositiveButton("OK", (dialog, which) -> dialog.dismiss());
        AlertDialog alertDialog = builder.create();
        alertDialog.setCanceledOnTouchOutside(false);
        alertDialog.show();
    }

    private void setImmersiveSticky() {
        getWindow()
                .getDecorView()
                .setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }

    // native interfaces
    private native long nativeOnCreate();
    private native void nativeOnDestroy(long nativeApp);
    private native void nativeOnPause(long nativeApp);
    private native void nativeOnResume(long nativeApp);
    private native void nativeOnSwitchViewer(long nativeApp);
}
