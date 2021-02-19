package com.example.ubiformandroidstreamingexample

import android.content.Intent
import android.net.Uri
import android.net.wifi.WifiManager
import android.os.Build
import android.os.Bundle
import android.os.Parcel
import android.os.ParcelFileDescriptor
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.io.BufferedReader
import java.io.InputStreamReader

class MainActivity : AppCompatActivity() {
    val PICKFILE_RESULT_CODE = 1
    private var fileUri: Uri? = null
    private var mParcelFileDescriptor : ParcelFileDescriptor? = null


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val wifiManager = applicationContext.getSystemService(WIFI_SERVICE) as WifiManager
        val numAddress = wifiManager.connectionInfo.ipAddress.toLong()
        val ipAddress =
            "tcp://${(numAddress and 0xff)}.${(numAddress shr 8 and 0xff)}.${(numAddress shr 16 and 0xff)}.${(numAddress shr 24 and 0xff)}"

        val componentMsg = startComponent(ipAddress, this)
        Log.d("UbiForm", ipAddress)
    }

    fun fileButtonClick(view: View){
        var chooseFile = Intent(Intent.ACTION_GET_CONTENT)
        chooseFile.type = "*/*"
        chooseFile = Intent.createChooser(chooseFile, "Choose a file")
        startActivityForResult(chooseFile, PICKFILE_RESULT_CODE)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        when (requestCode) {
            PICKFILE_RESULT_CODE -> {
                val text = findViewById<TextView>(R.id.file_path_text)
                if (resultCode == -1) {
                    fileUri = data?.data


                    mParcelFileDescriptor = fileUri?.let {
                        contentResolver.openFileDescriptor(it, "r")
                    }

                    val fd: Int? = mParcelFileDescriptor?.fd
                    if(fd != null){
                        text.text = openFile("/proc/self/fd/$fd")
                    }else{
                        text.text = "Unable to open the file descriptor"
                    }
                }
            }
        }
    }

    fun updateMainOutput(text: String) {
        val out = findViewById<TextView>(R.id.file_path_text)
        out.post { out.text = text }
    }

    override fun onDestroy() {
        super.onDestroy()
        deleteComponent()
    }

    external fun deleteComponent()
    external fun startComponent(ipAddress: String, activityObject: MainActivity)
    external fun openFile(fileLoc: String) : String

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}