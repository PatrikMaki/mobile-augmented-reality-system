package com.example.camerademo1

import Client
import android.annotation.SuppressLint
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.graphics.RectF
import android.os.Bundle
import android.util.Log
import android.view.SurfaceView
import android.view.TextureView
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.RadioButton
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.example.camerademo1.localProcessor.LocalProcessor
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

/*
Main class of the MARS Android application.
Runs the main UI thread, so Blocking this thread is
not allowed!
 */
class MainActivity : AppCompatActivity() {

    private val camera = CAL() //Camera Abstraction Layer
    private val client = Client("84.248.82.180" ) { //change this to correct value, TODO: change to var and update doc
        textureViewColor.post {
            camera.startCameraSession(this, textureViewColor) {
                    image->callbackClientSender(image)
            }
        }
    }
    private val localProcessor = LocalProcessor()
    lateinit var textureViewColor: SurfaceView // live camera view
    lateinit var textureViewColor2: TextureView // inference video view
    lateinit var textOutput: TextView // statistics output
    lateinit var textOutput2: TextView // average statistics output
    lateinit var textInput: EditText //TODO: use for IP input
    // resolutions buttons:
    lateinit var reso1: RadioButton
    lateinit var reso2: RadioButton
    lateinit var reso3: RadioButton
    lateinit var reso4: RadioButton
    lateinit var ipButton: Button //TODO: IP Button
    private lateinit var matrix: Matrix

    @SuppressLint("ResourceType")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        // Request camera permissions
        if (allPermissionsGranted()) {
            try {
                /*
                 Load model file for inference, if they are found.
                 TODO: Add loading file from a remote DB server!
                 */
                assetFilePath(this, "model.pt")?.let {
                    assetFilePath(this, "labels.txt")?.let {
                            it1 -> localProcessor.loadModel(it, it1)
                    }
                }
            } catch (e: Exception) {
                Log.e("PytorchHelloWorld", "Error reading assets", e);
                finish();
            }
            client.start()
            localProcessor.start()
            textureViewColor = findViewById<SurfaceView>(R.id.textureViewColor)
            textureViewColor.post { camera.startPreview(this, textureViewColor) }
            textureViewColor2 = findViewById<TextureView>(R.id.textureViewColor2)
            textOutput = findViewById<TextView>(R.id.textOutput)
            textOutput2 = findViewById<TextView>(R.id.textOutput2)
            //textInput = findViewById<EditText>(R.id.textInput)
            reso1 = findViewById<RadioButton>(R.id.reso1)
            reso2 = findViewById<RadioButton>(R.id.reso2)
            reso3 = findViewById<RadioButton>(R.id.reso3)
            reso4 = findViewById<RadioButton>(R.id.reso4)
            //ipButton = findViewById<Button>(R.id.ip_button)

            reso1.setOnClickListener {
                changeResolution(320, 240);
            }
            reso2.setOnClickListener {
                changeResolution(640, 480);
            }
            reso3.setOnClickListener {
                changeResolution(1280, 720);
            }
            reso4.setOnClickListener {
                changeResolution(1920, 1080);
            }
            /*
            TODO:
            ipButton.setOnClickListener{
                client.changeIP(textInput.text.toString())
                client.disconnect()
                client.connect()
            }*/
        } else {
            ActivityCompat.requestPermissions(
                this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS
            )
        }
        findViewById<Button>(R.id.stream_button).apply {
            setOnClickListener { startStream() }
        }
        findViewById<Button>(R.id.stream_button2).apply {
            setOnClickListener { startLocalStream() }
        }
        findViewById<Button>(R.id.stop_stream_button).apply {
            setOnClickListener { stopStream() }
        }
    }
    // Application state
    enum class StateEnum {
        StateIdle,
        StatePreview,
        StateStream
    }
    var state : StateEnum = StateEnum.StateIdle
    private fun changeResolution(width: Int, height: Int) {
        camera.WIDTH = width
        camera.HEIGHT = height
        if (state == StateEnum.StateStream) {
            startStream()
        } else if (state == StateEnum.StatePreview) {
            textureViewColor.post { camera.startPreview(this, textureViewColor) }
        }
    }

    //Stops stream and restarts preview.
    private fun  stopStream() {
        camera.stopStream()
        textureViewColor.post { camera.startPreview(this, textureViewColor) }
        state = StateEnum.StatePreview
    }

    //Stops preview and starts stream.
    private fun  startStream() {
        camera.stopPreview()
        client.attempt("connect")
        client.attempt("startStream")
        showVideo()
        state = StateEnum.StateStream
    }

    //Stops preview and starts local mode.
    private fun startLocalStream() {
        println("before stoppping stream or preview")
        camera.stopStream() //?
        camera.stopPreview()
        textureViewColor.post {
            camera.startCameraSession(this, textureViewColor) {
                    image->callbackLocal(image)
            }
        }
        showVideoLocalProcessing()
    }

    /*
    Sends the JPG images from the Camera to the server with TCP.
     */
    private fun callbackClientSender(receivedmsg: Frame) {
        client.send(receivedmsg)
    }

    /*
    Sends the JPG images from the camera to a local queue.
     */
    private fun callbackLocal(receivedmsg: Frame) {
        localProcessor.enqueue(receivedmsg)
    }

    //OBSOLETE:
    private fun updateTransform(textureView: View, rotationDegrees: Float) : Matrix {
        val matrix = Matrix()

        val centerX = textureView.width / 2f
        val centerY = textureView.height / 2f

        val bufferRect = RectF(0f, 0f, camera.WIDTH.toFloat(), camera.HEIGHT.toFloat())
        val viewRect = RectF(0f, 0f, textureView.width.toFloat(), textureView.height.toFloat())
        matrix.setRectToRect(bufferRect, viewRect, Matrix.ScaleToFit.FILL)
        matrix.postRotate(rotationDegrees, centerX, centerY)

        return matrix
    }

    private fun updateTransform2(textureView: View, rotationDegrees: Float) : Matrix {
        if (this::matrix.isInitialized) {
            return matrix
        }
        matrix = Matrix()
        return matrix
        val centerX = textureView.width / 2f
        val centerY = textureView.height / 2f

        val bufferRect = RectF(0f, 0f, camera.WIDTH.toFloat(), camera.HEIGHT.toFloat())
        val viewRect = RectF(0f, 0f, textureView.height.toFloat(), textureView.width.toFloat())
        matrix.setRectToRect(bufferRect, viewRect, Matrix.ScaleToFit.FILL)
        matrix.postRotate(rotationDegrees, centerX, centerY)

        return matrix
    }

    /*
    Starts Thread for receiving, processing and showing
    locally processed frames.
    Also, displays statistics.
     */
    private fun showVideoLocalProcessing() {
        Thread.sleep(1000)
        Thread(Runnable{
            var i = 0
            var time = System.currentTimeMillis()
            var timeSum: Long = 0
            var displayTime: Long = System.currentTimeMillis()
            var displayInterval: Long = 5000
            var processTimeSum: Long = 0
            val testStartTime: Long = System.currentTimeMillis()
            while(true) {
                val image = localProcessor.receive()
                val canvas = textureViewColor2.lockCanvas()
                if (canvas != null) {
                    canvas.drawBitmap(image.bitmap,updateTransform2(textureViewColor2, 90f), null)
                    textureViewColor2.unlockCanvasAndPost(canvas)
                }
                i++
                val now = System.currentTimeMillis()
                val timeDelta = now - image.timeStart
                timeSum += timeDelta
                processTimeSum += image.processTime

                if (i==30) {
                    i=0
                    val time2 = now
                    println("local Q=${localProcessor.getQueueLength()} #${image.frameCount} t=${time2-image.timeStart} pt=${image.processTime} fps=${String.format("%.2f", (30.0*1000/(time2-time)))}")
                    textOutput.text = "local Q=${localProcessor.getQueueLength()} #${image.frameCount} t=${time2-image.timeStart} pt=${image.processTime} fps=${String.format("%.2f", (30.0*1000/(time2-time)))}"
                    time = time2
                }
                if (now-displayTime > displayInterval){
                    println("local avg t=${timeSum/image.frameCount} avg pt=${processTimeSum/image.frameCount} avg fps=${String.format("%.2f",1000.0*image.frameCount/(now-testStartTime))}")
                    textOutput2.text = "local avg t=${timeSum/image.frameCount} avg pt=${processTimeSum/image.frameCount} avg fps=${String.format("%.2f",1000.0*image.frameCount/(now-testStartTime))}"
                    displayTime = now
                }
            }
        }).start()
    }

    /*
    Starts Thread for receiving, processing and showing
    remotely processed frames.
    Also, displays statistics.
     */
    private fun showVideo() {
        Thread.sleep(1000)
        Thread(Runnable{
            var i = 0
            var time = System.currentTimeMillis()
            var timeSum: Long = 0
            var displayTime: Long = System.currentTimeMillis()
            var displayInterval: Long = 5000
            var processTimeSum: Long = 0
            val testStartTime: Long = System.currentTimeMillis()
            while(true) {
                val imageBuf = client.receive()
                val image = BitmapFactory.decodeByteArray(imageBuf.buffer, 0, imageBuf.buffer.size)
                val canvas = textureViewColor2.lockCanvas()
                if (canvas != null) {
                    canvas.drawBitmap(image, updateTransform2(textureViewColor2, 90f), null)
                    textureViewColor2.unlockCanvasAndPost(canvas)
                }
                i++
                val now = System.currentTimeMillis()
                val timeDelta = now - imageBuf.timeStart
                timeSum += timeDelta
                processTimeSum += imageBuf.processTime
                if (i==30) {
                    i=0
                    val time2 = System.currentTimeMillis()
                    println("Remote Lag=${globalframeCounter-imageBuf.frameCount} #${imageBuf.frameCount} t=${time2-imageBuf.timeStart} pt=${imageBuf.processTime} fps=${String.format("%.2f", (30.0*1000/(time2-time)))}")
                    textOutput.text = "Remote Lag=${globalframeCounter-imageBuf.frameCount} #${imageBuf.frameCount} t=${time2-imageBuf.timeStart} pt=${imageBuf.processTime} fps=${String.format("%.2f", (30.0*1000/(time2-time)))}"
                    time = time2
                }
                if (now-displayTime > displayInterval && imageBuf.frameCount>0){
                    println("Remote avg t=${timeSum/imageBuf.frameCount} avg pt=${processTimeSum/imageBuf.frameCount} avg fps=${String.format("%.2f",1000.0*imageBuf.frameCount/(now-testStartTime+1))}")
                    textOutput2.text = "Remote avg t=${timeSum/imageBuf.frameCount} avg pt=${processTimeSum/imageBuf.frameCount} avg fps=${String.format("%.2f",1000.0*imageBuf.frameCount/(now-testStartTime+1))}"
                    displayTime = now
                }
            }
        }).start()
    }

    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(
            baseContext, it) == PackageManager.PERMISSION_GRANTED
    }

    @Throws(IOException::class)
    fun assetFilePath(context: Context, assetName: String?): String? {
        val file = File(context.filesDir, assetName)
        if (file.exists() && file.length() > 0) {
            return file.absolutePath
        }
        context.assets.open(assetName!!).use { `is` ->
            FileOutputStream(file).use { os ->
                val buffer = ByteArray(4 * 1024)
                var read: Int
                while (`is`.read(buffer).also { read = it } != -1) {
                    os.write(buffer, 0, read)
                }
                os.flush()
            }
            return file.absolutePath
        }
    }
}