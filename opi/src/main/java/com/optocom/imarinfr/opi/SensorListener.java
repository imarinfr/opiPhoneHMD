package com.optocom.imarinfr.opi;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class SensorListener implements SensorEventListener {

    private final SensorManager sensorManager;
    private final Sensor sensor;

    float light = -1;

    SensorListener(Context context) {
        sensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        light = event.values[0];
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    public SensorManager getSensorManager() {
        return sensorManager;
    }

    public Sensor getSensor() {
        return sensor;
    }

    public float getLight() {
        return light;
    }
}