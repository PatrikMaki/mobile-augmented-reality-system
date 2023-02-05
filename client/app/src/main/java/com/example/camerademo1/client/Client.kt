import com.example.camerademo1.Frame
import java.util.*
import java.util.concurrent.LinkedBlockingDeque

var sendtimes: List<Long> = ArrayList<Long>()
var receivetimes: List<Long> = ArrayList<Long>()
var i = 0
var flag_n = true
var n = 10
var runningStreamGlobal = false
var runningControlGlobal = false
var localProcessing = false
//various ip addresses
//val globalIP = "172.17.144.1"
//val globalIP = "192.168.1.33"
//val globalIP = "172.17.159.73"
//var xrunning = false
var runningEdge = false
//val globalIP = "172.21.191.100"
//val globalIP = "127.0.0.1"
//val globalIP = "192.168.1.33"
var globalIP = "172.18.149.219" //TODO: can be removed

open class Client(ip: String , callback: () -> Unit): Thread() {
    var callbackStreamReady = callback
    var queue = LinkedBlockingDeque<String>()
    private lateinit var control: ControlProtocol
    private lateinit var stream: StreamProtocol

    //Thread:
    private lateinit var receiveControlThread: Thread
    private lateinit var streamThread: Thread


    init {
        globalIP = ip;
    }

    private fun handleRequest(request: String): String {
        if("tcpjpg" == request) {
            runningStreamGlobal = true
            stream = StreamProtocol(globalIP,4321, "tcpjpg")
            callbackStreamReady()
            return "OK"
        } else {
            return "Error"
        }
    }

    fun changeIP(ip: String) {
        globalIP = ip;
    }
    private fun connect() { //could return something //thread
        control = ControlProtocol(globalIP,4231) //is implemented
        runningControlGlobal = true
        receiveControlThread = Thread { control.rec() }
        receiveControlThread.start()
        control.setCallBack { request: String -> handleRequest(request) } //is implemented
        control.hello()
    }
    private fun startStream() { //thread
        runningEdge = true
        control.start("tcpjpg classify") //async response comes from server goes to handleRequest
        Thread.sleep(20)
        this.callbackStreamReady = callbackStreamReady
    }

    fun send(msg: Frame) {//ByteArray
        //stream.send(msg) //send image with time stamp
        stream.send(msg)
    }

    fun receive(): Frame {
        return stream.receive() //receive image with time stamp
    }
    fun getQin(): Int {
        return stream.getQin()
    }
    fun getQout(): Int {
        return stream.getQout()
    }
    private fun stopStream() { //thread
        runningEdge = true
        control.stopp()
    }
    private fun disconnect() { //thread
        control.stopp() //not tested
        control.close() //not tested
        runningControlGlobal = false //stops the thread
    }
    override fun run() {
        while(true) {
            val request = queue.takeLast()
            when(request) {
                "connect" -> connect()
                "startStream" -> startStream()
                "stopStream" -> stopStream()
                "disconnect" -> disconnect()
            }
        }
    }
    fun attempt(request: String) {
        queue.addFirst(request)
    }
}