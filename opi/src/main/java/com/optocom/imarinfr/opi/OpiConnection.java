package com.optocom.imarinfr.opi;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Enumeration;
import java.lang.String;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.os.Build;
import android.util.DisplayMetrics;

import androidx.annotation.RequiresApi;

public class OpiConnection extends Thread {
    private static final int LOCALPORT = 50008;

    private static final String OPI_GET_METRICS      = "OPI_GET_METRICS";
    public  static final String OPI_SET_BACKGROUND   = "OPI_SET_BACKGROUND";
    public  static final String OPI_PRESENT          = "OPI_PRESENT";
    private static final String OPI_CLOSE            = "OPI_CLOSE";
    private static final String OK                   = "OK";

    private final Context context;
    private final long nativeApp;
    private final Renderer renderer;
    private final SensorListener sensorListener;

    private BufferedReader in;
    private BufferedWriter out;

    public OpiConnection(Context mainContext, long mainNativeApp, Renderer mainRenderer,
                         SensorListener mainSensorListener) {
        context        = mainContext;
        nativeApp      = mainNativeApp;
        renderer       = mainRenderer;
        sensorListener = mainSensorListener;

        start();
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void run() {
        String msg, cmd;    // strings for message received and command
        String pars;        // string array for parameters passed with command, if any
        boolean opiOpened = false;
        try {
            ServerSocket server = new ServerSocket(LOCALPORT, 1);
            Socket client;
            //noinspection InfiniteLoopStatement
            do {
                if (!opiOpened) {
                    client = server.accept();
                    in = new BufferedReader(new InputStreamReader(client.getInputStream()));
                    out = new BufferedWriter(new OutputStreamWriter(client.getOutputStream()));
                    opiOpened = true;
                } else {
                    msg = in.readLine(); // read message
                    pars = "";
                    // if no parameters are found
                    if (!msg.contains(" ")) cmd = msg;
                    else {
                        cmd = msg.substring(0, msg.indexOf(" "));
                        pars = msg.substring(msg.indexOf(" ") + 1); // get params, if any
                    }
                    if (cmd.equals(OPI_CLOSE)) { // close OPI connection
                        opiClose();
                        opiOpened = false;
                    } else processCommand(cmd, pars.split(" ")); // perform command
                }
            } while (true);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    public void processCommand(String cmd, String[] pars) {
        switch(cmd) {
            case OPI_GET_METRICS:
                opiGetMetrics();
                break;
            case OPI_SET_BACKGROUND:
                opiSetBackground(pars);
                break;
            case OPI_PRESENT:
                opiPresent(pars);
                break;
            default:
                break;
        }
    }

    public String socketAddress() {
        if(getIPAddress() != null)
            return getIPAddress() + ":" + LOCALPORT;
        else return null;
    }

    private String getIPAddress() {
        ConnectivityManager cm = ((ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE));
        NetworkCapabilities cap = cm.getNetworkCapabilities(cm.getActiveNetwork());
        if (cap != null && cap.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)) {
            try {
                for(Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements(); ) {
                    NetworkInterface networkInterface = en.nextElement();
                    for(Enumeration<InetAddress> enumIpAddr = networkInterface.getInetAddresses();
                        enumIpAddr.hasMoreElements(); ) {
                        InetAddress inetAddress = enumIpAddr.nextElement();
                        if(!inetAddress.isLoopbackAddress() && inetAddress instanceof Inet4Address) {
                            return inetAddress.getHostAddress();
                        }
                    }
                }
            } catch(SocketException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    public void opiGetMetrics() {
        // TODO find min and max luminance of the phone and report back to R OPI
        DisplayMetrics displayMetrics = new DisplayMetrics();
        context.getDisplay().getRealMetrics(displayMetrics);
        writeMsg(String.valueOf(displayMetrics.widthPixels)); // send phone metrics
        writeMsg(String.valueOf(displayMetrics.heightPixels));
        writeMsg(String.valueOf(displayMetrics.xdpi));
        writeMsg(String.valueOf(displayMetrics.ydpi));
        float[] fov = nativeGetFieldOfView(nativeApp); // get field of view
        writeMsg(String.valueOf(fov[0]));
        writeMsg(String.valueOf(fov[1]));
        writeMsg(String.valueOf(fov[2]));
        writeMsg(String.valueOf(fov[3]));
        writeMsg(Float.toString(sensorListener.getLight()));
    }

    private void opiClose() {
        renderer.changeBackground(new Background());
        writeMsg(OK);
    }
    public void opiSetBackground(String[] pars) {
        Background bg = new Background();
        boolean done = bg.parseParameters(pars);
        if(done) {
            renderer.changeBackground(bg);
            writeMsg(OK);
        } else writeMsg("OPI server: Background parameters are not valid");
    }

    public void opiPresent(String[] pars) {
        boolean trialRunning = true;
        long time;
        String seen;
        String errorMsg = "";
        // parse global parameters
        Stimulus stim = new Stimulus();
        boolean correct = stim.parseGlobalPars(pars);
        if(correct) {
            // if correct, inform client, then proceed
            // to parse step parameters
            writeMsg(OK);
            for(int step = 0; step < stim.steps(); step++) {
                try {
                    correct = stim.parseStepPars(step, in.readLine().split(" "));
                } catch (IOException e) {
                    correct = false;
                }
                if(correct) writeMsg(OK);
                else break;
            }
            // if still correct, inform client, then present
            if(correct) {
                renderer.presentStimulus(stim);
                // block until at least minimum presentation time stim.d has passed
                while(trialRunning) trialRunning = renderer.trialRunning();
                time = renderer.responseTime();
                if(time > 0) seen  = "true";
                else         seen  = "false";
                sendResults("", seen, Long.toString(time));
            } else errorMsg = "OPI server: Step parameters are not valid";
        } else errorMsg = "OPI server: Global stimulus parameters are not valid";
        if(!correct) writeMsg(errorMsg);
    }

    public void sendResults(String err, String seen, String time){
        writeMsg(err);
        writeMsg(seen);
        writeMsg(time);
    }

    private void writeMsg(String txt) {
        try {
            out.write(txt);
            out.newLine();
            out.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private native float[] nativeGetFieldOfView(long nativeApp);
}