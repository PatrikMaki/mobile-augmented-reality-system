package com.example.camerademo1.localProcessor


import android.graphics.*
import com.example.camerademo1.Frame
import org.pytorch.*
import org.pytorch.torchvision.TensorImageUtils
import java.io.File
import java.util.concurrent.LinkedBlockingDeque

//https://pytorch.org/mobile/android/
fun loadLabels(filePath: String): Array<String> {
    var list: MutableList<String> = mutableListOf()
    File(filePath).forEachLine {list.add(it)}
    return list.toTypedArray()
}
fun drawTextToBitmap(
    bitmapin: Bitmap,
    gText: String
): Bitmap? {
    var bitmapConfig = bitmapin.config
    // set default bitmap config if none
    if (bitmapConfig == null) {
        bitmapConfig = Bitmap.Config.ARGB_8888
    }
    // resource bitmaps are imutable,
    // so we need to convert it to mutable one
    var bitmap = bitmapin.copy(bitmapConfig, true);

    val canvas = Canvas(bitmap)
    // new antialised Paint
    val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    // text color - #3D3D3D
    paint.setColor(Color.rgb(61, 61, 61))
    // text size in pixels
    val scale = 2
    paint.setTextSize((14 * scale).toInt().toFloat())
    // text shadow
    paint.setShadowLayer(1f, 0f, 1f, Color.WHITE)

    // draw text to the Canvas center
    val bounds = Rect()
    paint.getTextBounds(gText, 0, gText.length, bounds)
    val x: Int = (bitmap.width - bounds.width()) / 2
    val y: Int = (bitmap.height + bounds.height()) / 2
    canvas.drawText(gText, x.toFloat(), y.toFloat(), paint)
    return bitmap
}
class LocalProcessor (callback: () -> Unit): Thread(){
    //queue for images,
    //model preparation
    //inference
    var callback = callback
    var queue = LinkedBlockingDeque<Frame>()
    var queueDone = LinkedBlockingDeque<Frame>()
    // loading serialized torchscript module from packaged into app android asset model.pt,
    // app/src/model/assets/model.pt
    // loading serialized torchscript module from packaged into app android asset model.pt,
    // app/src/model/assets/model.pt
    private lateinit var  module: Module
    private lateinit var labels: Array<String>
    fun loadModel(path: String, labelPath: String) {
        module = LiteModuleLoader.load(path)
        labels = loadLabels(labelPath)
    }

    fun getQueueLength(): Int {
        return queue.size
    }
    fun enqueue(buf: Frame) {
        queue.addFirst(buf)
    }
    override fun run() {
        while (true) {
            val a = queue.takeLast()
            val timeBefore = System.currentTimeMillis()
            val b = inference(a.buffer) //calculate inference time
            val timeAfter = System.currentTimeMillis()
            val frame = Frame()
            frame.frameCount = a.frameCount
            if (b != null) {
                frame.bitmap = b
            }
            frame.timeStart = a.timeStart
            frame.processTime = timeAfter -timeBefore
            queueDone.addFirst(frame)
        }
    }
    fun receive(): Frame {
        return queueDone.takeLast()
    }
    private fun inference(img: ByteArray): Bitmap? {
        //img[400] = 254.toByte()
        // preparing input tensor

        // preparing input tensor
        val bitmap = BitmapFactory.decodeByteArray(img, 0, img.size)
        val inputTensor = TensorImageUtils.bitmapToFloat32Tensor(
            bitmap,
            TensorImageUtils.TORCHVISION_NORM_MEAN_RGB,
            TensorImageUtils.TORCHVISION_NORM_STD_RGB,
            MemoryFormat.CHANNELS_LAST
        )

        // running the model

        // running the model
        val outputTensor = module.forward(IValue.from(inputTensor)).toTensor()

        // getting tensor content as java array of floats

        // getting tensor content as java array of floats
        val scores = outputTensor.dataAsFloatArray

        // searching for the index with maximum score

        // searching for the index with maximum score
        var maxScore = -Float.MAX_VALUE
        var maxScoreIdx = -1
        for (i in scores.indices) {
            if (scores[i] > maxScore) {
                maxScore = scores[i]
                maxScoreIdx = i
            }
        }
        val className = labels[maxScoreIdx]
        return drawTextToBitmap(bitmap, className)
    }
}