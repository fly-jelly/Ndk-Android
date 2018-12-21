package com.deepcam.mtcnn;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

import com.deepcam.detect.natives.FaceDetectNative;

import java.util.ArrayList;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
//        tv.setText(String.format("code : %d",init()));
        FaceDetectNative fdn = FaceDetectNative.instance();
        ArrayList face = new ArrayList();
        face.add(12.f);

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
  /*  public native int init();

    public native void close();*/
}
