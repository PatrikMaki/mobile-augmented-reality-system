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

class Frame() {
    var frameCount: Long = 0
    var timeStart: Long = 0
    var timeEnd: Long = 0 //not mandatory
    var processTime: Long = 0
    lateinit var buffer: ByteArray
    lateinit var bitmap: Bitmap
}
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
/*
        val imgInfo =
            ImageInfo(WIDTH, HEIGHT, 16, false, true, false)

        cameraManager.openCamera(depthCameraId, object : CameraDevice.StateCallback() {
            override fun onDisconnected(device: CameraDevice) {
                device.close()
            }
            override fun onError(device: CameraDevice, error: Int) {
                onDisconnected(device)
            }

            override fun onOpened(cameraDevice: CameraDevice) {
//              ************************************************* RAW Image Reader *************************************************
                val depthPreviewReader = ImageReader.newInstance(WIDTH, HEIGHT, ImageFormat.DEPTH16, 5)
                depthPreviewReader.setOnImageAvailableListener({ reader ->
                    val image = reader.acquireNextImage()

                    val w = image.width
                    val h = image.height

//                  Create 8 bit grayscale depth bitmap
//                    val buffer: ByteBuffer = image.planes[0].buffer
//                    val bytes = ByteArray(buffer.remaining())
//                    buffer[bytes]

//                    val tempDepth = Base64.getEncoder().encode(bytes)
//                    val start = System.currentTimeMillis()
//                    val bitmapDepth = readDepthBitmap(image)
//                    Log.i(sentframes.toString(), (System.currentTimeMillis() - start).toString())

                    val output = readDepthValues(image)

//                  Read depth range and confidence values
//                    val bitmapDepth = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
//                    val bitmapConfidence = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
//
//                    var offset = 0
//                    for (y in 0 until h) {
//                        for (x in 0 until w) {
//
//                            val colorDepth = (output[offset + x] / DEPTH_RANGE_FILTER * 255).toInt()
//                            bitmapDepth.setPixel(x, y, Color.rgb(colorDepth, colorDepth, colorDepth))
//
//                            val colorConf = (confidenceValues[offset + x] * 255).toInt()
//                            bitmapConfidence.setPixel(x, y, Color.rgb(colorConf, colorConf, 0))
//                        }
//                        offset += w
//                    }



                    if (streaming) {
                        try {
//                      Write 16 bit depth PNG
                            Thread(Runnable {
                                var byteArrayOutputStream = ByteArrayOutputStream(w * h)

                                val pngWriter = PngWriter(byteArrayOutputStream, imgInfo)

                                var offset = 0
                                for (i in 0 until imgInfo.rows) {
                                    val imgLine = ImageLineInt(imgInfo)

                                    for (j in 0 until imgInfo.cols) {
                                        imgLine.scanline[j] = output[offset + j].toInt()
                                    }
                                    offset += imgInfo.cols
                                    pngWriter.writeRow(imgLine)
                                }
                                pngWriter.end()

                                sentframes++
                                val size = byteArrayOutputStream.toByteArray()
                                val depth16 = Base64.getEncoder().encode(byteArrayOutputStream.toByteArray())

                                val obj = JSONObject()
                                obj.put("rgb", rgbBytes)
                                obj.put("depth", depth16)
                                obj.put("frame", sentframes)
                                obj.put("time", System.currentTimeMillis().toString())


                                Log.i("", "---------" + size.count().toString() + " " + depth16.count().toString())

                                socket.emit("rgbd", obj)

//                                socket.emit("starting", System.currentTimeMillis().toString())
//                                socket.on("pong starting",onPong)
                            }).start()

                        } catch (e: IOException) {
                            e.printStackTrace()
                        } finally {
                            image.close()
                        }
                    }
                    else
                        image.close()

//                    if (streaming) {
//                        Thread(Runnable {
//                            try {

//                                //webp part
//                                val wbaos = ByteArrayOutputStream()
//                                bitmapDepth.compress(Bitmap.CompressFormat.WEBP, 100, wbaos)
//                                val wdepthBytes = Base64.getEncoder().encode(wbaos.toByteArray())
//
//
//                                val obj = JSONObject()
//                                obj.put("rgb", rgbBytes)
//                                obj.put("depth", wdepthBytes)
//                                obj.put("frame", sentframes++)
//
//                                socket.emit("rgbd", obj)
//                            }
//                            catch (e: URISyntaxException) {
//                                Log.i("ex", e.toString())
//                            }
//                        }).start()
//                    }

//                    renderBitmapToTextureView(bitmapDepth, textureViewDepth)

                }, null)


                val captureCallback: CameraCaptureSession.StateCallback = object : CameraCaptureSession.StateCallback() {
                    override fun onConfigureFailed(session: CameraCaptureSession) {}

                    override fun onConfigured(session: CameraCaptureSession) {
//                        depthCameraCaptureSession = session

                        // preview
                        val previewRequestBuilder =
                            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
                        previewRequestBuilder.addTarget(depthPreviewReader.surface)

                        session.setRepeatingRequest(
                            previewRequestBuilder.build(),
                            object : CameraCaptureSession.CaptureCallback() {},
                            Handler { true }
                        )
                    }
                }
                cameraDevice.createCaptureSession(
                    mutableListOf(depthPreviewReader.surface),
                    captureCallback,
                    Handler { true })

            }
        }, Handler { true })
*/

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
                    //buffer[bytes] //does nothing
                    /*
                    var output: FileOutputStream? = null

                    try {
                        val file = File(externalMediaDirs.first(), "${currentFileName}.jpg")

                        output = FileOutputStream(file)
                        output.write(bytes)
                    } catch (e: IOException) {
                        e.printStackTrace()
                    } finally {
                        image.close()
                    }*/ //we don't save to file here
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
                    //myCallback.invoke(bytes)

                    //rgbBytes = Base64.getEncoder().encode(bytes)

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

        //done elsewhere
        /*
        activity.findViewById<Button>(R.id.stream_button).setOnClickListener {
            currentFileName = System.currentTimeMillis().toString()

            val socket
            if (socket.connected()) {
                txtSocket.text = "Connected"

                streaming = !streaming
                if (streaming) {
                    startTime = System.currentTimeMillis()
                    socket.emit("starting", System.currentTimeMillis().toString())
                    socket.on("pong starting", onPong)

                    txtStreaming.text = "Streaming"
                    Toast.makeText(
                        context,
                        "Streaming",
                        Toast.LENGTH_SHORT
                    ).show()
                }
                else
                    txtStreaming.text = "Not streaming"
            }
            else {
                txtSocket.text = "Not connected"
                Toast.makeText(
                    context,
                    "Not connected",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }*/
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