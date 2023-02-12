package com.example.camerademo1

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.ImageFormat
import android.hardware.camera2.*
import android.media.ImageReader
import android.os.Handler
import android.view.SurfaceView
import android.widget.Button
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat.getSystemService

import ar.com.hjg.pngj.ImageInfo
import java.nio.Buffer
import java.nio.ByteBuffer
import java.util.*

private const val LENS_FACING = CameraMetadata.LENS_FACING_BACK
val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
const val REQUEST_CODE_PERMISSIONS = 10

var globalframeCounter: Long = 0

/*
Class for storing an image frame, with metadata
 */
class Frame() {
    var frameCount: Long = 0 // increasing frame number
    var timeStart: Long = 0 // frame capture time in milliseconds
    var timeEnd: Long = 0 // not mandatory
    var processTime: Long = 0 // inference process time in milliseconds
    lateinit var buffer: ByteArray // storing the image
    lateinit var bitmap: Bitmap // storing the image in displayable format
}

/*
Camera Abstraction Layer
- makes using the Android camera2 API easier
 */
class CAL() {
    //val WIDTH = 240
    //val HEIGHT = 180
    //val WIDTH = 320
    //val HEIGHT = 240
    var WIDTH = 480
    var HEIGHT = 640
    //var WIDTH = 1280
    //var HEIGHT = 720
    //val WIDTH = 1920
    //val HEIGHT = 1080


    //val WIDTH = 3820 //doesn't work
    //val HEIGHT = 2160

    private lateinit var defaultCameraId: String
    private lateinit var depthCameraId: String
    private lateinit var colorCameraCaptureSession: CameraCaptureSession
    private lateinit var rgbBytes: ByteArray
    private lateinit var colorCaptureRequestBuilder: CaptureRequest.Builder

    fun stopPreview() {
        colorCameraCaptureSession.stopRepeating()
    }
    fun stopStream() {
        colorCameraCaptureSession.stopRepeating()
    }
    //preview with no effects
    fun startPreview(context: Context, textureViewColor: SurfaceView) {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            return
        }

        val cameraManager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
        if (cameraManager.cameraIdList.isEmpty()) {
            // no cameras
            return
        }

        initCameraIds(cameraManager, LENS_FACING)

        val cameraCharacteristics = cameraManager.getCameraCharacteristics(defaultCameraId)

        cameraCharacteristics[CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP]?.let { streamConfigurationMap ->
            streamConfigurationMap.getOutputSizes(ImageFormat.JPEG)?.let { jpegSizes ->
            }
        }

        cameraManager.openCamera(defaultCameraId, object : CameraDevice.StateCallback() {
            override fun onDisconnected(device: CameraDevice) {
                device.close()
            }
            override fun onError(device: CameraDevice, error: Int) {
                onDisconnected(device)
            }

            override fun onOpened(cameraDevice: CameraDevice) {
//              ************************************************* JPG Image Reader *************************************************


                val captureCallback: CameraCaptureSession.StateCallback = object : CameraCaptureSession.StateCallback() {
                    override fun onConfigureFailed(session: CameraCaptureSession) {}

                    override fun onConfigured(session: CameraCaptureSession) {
                        colorCameraCaptureSession = session

                        // preview
                        val previewRequestBuilder =
                            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
                        //previewRequestBuilder.addTarget(jpegStreamReader.surface)
                        previewRequestBuilder.addTarget(textureViewColor.holder.surface)

                        colorCameraCaptureSession.setRepeatingRequest(
                            previewRequestBuilder.build(),
                            object : CameraCaptureSession.CaptureCallback() {},
                            Handler { true }
                        )

                        // capture
                        colorCaptureRequestBuilder =
                            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE)
                        //colorCaptureRequestBuilder.addTarget(jpegCaptureReader.surface)
                    }
                }
                cameraDevice.createCaptureSession(
                    mutableListOf(textureViewColor.holder.surface),//, jpegStreamReader.surface, jpegCaptureReader.surface),
                    captureCallback,
                    Handler { true })
            }
        }, Handler { true })

    }

    //preview with callback with ByteArray
    fun startCameraSession(context: Context, textureViewColor: SurfaceView,  myCallback: (result: Frame) -> Unit) {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            return
        }

        val cameraManager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
        if (cameraManager.cameraIdList.isEmpty()) {
            // no cameras
            return
        }

        initCameraIds(cameraManager, LENS_FACING)

        var currentFileName = ""

        val cameraCharacteristics = cameraManager.getCameraCharacteristics(defaultCameraId)

        cameraCharacteristics[CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP]?.let { streamConfigurationMap ->
            streamConfigurationMap.getOutputSizes(ImageFormat.JPEG)?.let { jpegSizes ->
                val previewSize = jpegSizes.last()
            }
        }

        var streaming = false
        var sentframes = 0

        cameraManager.openCamera(defaultCameraId, object : CameraDevice.StateCallback() {
            override fun onDisconnected(device: CameraDevice) {
                device.close()
            }
            override fun onError(device: CameraDevice, error: Int) {
                onDisconnected(device)
            }

            override fun onOpened(cameraDevice: CameraDevice) {
//              ************************************************* JPG Image Reader *************************************************
                val jpegCaptureReader = ImageReader.newInstance(WIDTH, HEIGHT, ImageFormat.JPEG, 1)
                jpegCaptureReader.setOnImageAvailableListener({ reader ->
                    val image = reader.acquireNextImage()
                    val buffer: ByteBuffer = image.planes[0].buffer
                    val bytes = ByteArray(buffer.remaining())
                }, null)

                val jpegStreamReader = ImageReader.newInstance(WIDTH, HEIGHT, ImageFormat.JPEG, 1)
                /*
                //++ attempt to set jpeg compression ratio:
                val captureBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE) // TEMPLATE_PREVIEW
                captureBuilder.addTarget(jpegStreamReader.surface)
                captureBuilder.set(CaptureRequest.JPEG_QUALITY, 60)
                val captureRequest = captureBuilder.build() // This would require use of cameraDevice.createCaptureSession...
                //--
                 */
                jpegStreamReader.setOnImageAvailableListener({ reader ->
                    val image = reader.acquireNextImage() //time here
                    val buffer: ByteBuffer = image.planes[0].buffer
                    val bytes = ByteArray(buffer.remaining())
                    buffer[bytes] //we need image object with time stamp
                    val frame = Frame()
                    frame.timeStart = System.currentTimeMillis()
                    frame.frameCount = globalframeCounter++
                    frame.buffer = bytes
                    myCallback.invoke(frame)

                    image.close()
                }, null)


                val captureCallback: CameraCaptureSession.StateCallback = object : CameraCaptureSession.StateCallback() {
                    override fun onConfigureFailed(session: CameraCaptureSession) {}

                    override fun onConfigured(session: CameraCaptureSession) {
                        colorCameraCaptureSession = session

                        // preview
                        val previewRequestBuilder =
                            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
                        previewRequestBuilder.addTarget(jpegStreamReader.surface)
                        previewRequestBuilder.addTarget(textureViewColor.holder.surface)

                        colorCameraCaptureSession.setRepeatingRequest(
                            previewRequestBuilder.build(),
                            object : CameraCaptureSession.CaptureCallback() {},
                            Handler { true }
                        )

                        // capture
                        colorCaptureRequestBuilder =
                            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE)
                        colorCaptureRequestBuilder.addTarget(jpegCaptureReader.surface)
                    }
                }
                cameraDevice.createCaptureSession(
                    mutableListOf(textureViewColor.holder.surface, jpegStreamReader.surface, jpegCaptureReader.surface),
                    captureCallback,
                    Handler { true })
            }
        }, Handler { true })
    }

    private fun initCameraIds(cameraManager: CameraManager, lensFacing: Int) {
        for (camId in cameraManager.cameraIdList) {
            val characteristics = cameraManager.getCameraCharacteristics(camId)
            val capabilities =
                characteristics.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES)!!

            if (characteristics.get(CameraCharacteristics.LENS_FACING) == lensFacing) {

                if (capabilities.contains(CameraMetadata.REQUEST_AVAILABLE_CAPABILITIES_DEPTH_OUTPUT)) {
                    depthCameraId = camId
                    continue
                }

                // take the first front facing cameraId as recommended in documentation
                if (!this::defaultCameraId.isInitialized) {
                    defaultCameraId = camId
                    continue
                }
            }
        }
    }
}