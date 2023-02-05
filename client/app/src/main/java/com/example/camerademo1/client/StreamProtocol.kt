import com.example.camerademo1.Frame
import java.io.*
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.net.Socket
import java.util.concurrent.LinkedBlockingDeque


interface IstreamSend {
    fun send(n: Frame) //ByteArray
    fun stopp()
    fun getQs(): Int
}

interface IstreamReceive {
    fun getImageFromQueueu(): Frame
    fun stopp()
    fun getQr(): Int
}



private class Start_stream_test1_send(val ip: String, val port: Int, val socket: DatagramSocket): IstreamSend, Thread() {
    var queue = LinkedBlockingDeque<Frame>() //ByteArray
    var running = true
    init {
        this.start()
    }
    override fun send(n: Frame) { //ByteArray
        //println("#DEBUG: test1.send: $queue")
        queue.addFirst(n)
        //println("Queue size $queue.size()")
    }
    override fun stopp() {
        running = false
    }

    override fun getQs(): Int {
        return queue.size
    }

    override fun run() {
        //println("test1.send.run started")
        //println(running)
        while(running) {
            //println("queue.takeLast() $queue")
            try {
                val message = queue.takeLast()
                //println("sending message")
                //socket.broadcast = true
                var packet = DatagramPacket(message.buffer, message.buffer.size, InetAddress.getByName(ip), port)
                socket.send(packet)
                //println("Packet Sent")
            } catch (e: Exception) {
                println(e)
            }
        }
    }
}
private class Start_stream_test1_receive(val stream_type: String, val socket: DatagramSocket): IstreamReceive, Thread() {
    var running = true
    init {
        this.start()
    }
    override fun stopp() {
        //println("performance frames received:$i")
        i=0
        running = false
    }

    override fun getQr(): Int {
        return 0
    }

    override fun getImageFromQueueu(): Frame {
        val frame = Frame()
        frame.buffer= "not implemented".toByteArray()
        return frame
    }
    override fun run() {
        //println("test1.receive.run started")
        var buf: ByteArray = ByteArray(1500)
        while(running) {
            socket.broadcast = true
            var packet = DatagramPacket(buf, buf.size)
            socket.receive(packet)
            val c = packet.getLength()
            //println(c)
            val receivedBytes = packet.getData().copyOfRange(0,c)
            val a = receivedBytes.toString(Charsets.UTF_8)
            //println(receivedBytes)
            //println("vastaanotetaan:")
            println(a)
            i += 1
            //possible solutions:
            //callback
            //sleeps in send and receive
            //callback with and if no received packet in a certain time then send a new packet
            //independent receiver and sender while counting packet loss
        }
    }
}

private class Start_stream_tcpjpg_send(val ip: String, val port: Int, val client: Socket): IstreamSend, Thread() {
    var queue = LinkedBlockingDeque<Frame>() //ByteArray
    var running = true
    //private val client = Socket(ip, port)
    init {
        this.start()
    }
    override fun send(n: Frame) { //ByteArray
        //println("#DEBUG: test1.send: $queue")
        queue.addFirst(n)
        //println("Queue size $queue.size()")
    }
    override fun stopp() {
        running = false
    }

    override fun getQs(): Int {
        return queue.size
    }

    override fun run() {
        //println("test1.send.run started")
        //println(running)
        while(running) {
            //println("queue.takeLast() $queue")
            try {
                val message: Frame = queue.takeLast()
                //println("sending message")
                //socket.broadcast = true
                //var packet = DatagramPacket(message, message.size, InetAddress.getByName(ip), port)
                var hexbuf = String.format("img2 %016x %016x %016x %08x",
                    message.frameCount,
                    message.timeStart,
                    message.processTime,
                    message.buffer.size
                );
                //println("sending tcpjpg with hexbuf: " + hexbuf)

                // client.outputStream.write(("00000004aaaa").toByteArray())
                client.outputStream.write(hexbuf.toByteArray());
                client.outputStream.write(message.buffer)
                //println("Packet Sent")
            } catch (e: Exception) {
                println(e)
            }
        }
    }
}
var j = 0
private fun helper(buffer: ByteArray) { //TODO: can be removed
    receivetimes += System.currentTimeMillis()

    Thread(Runnable{
        j+=1
        try {
            val myObj = File("/Users/patrik/MARS/client/local/console/kotlinNetworking/images/write/apple${j}.jpg")
            if (myObj.createNewFile()) {
                println("File created: " + myObj.name)
                File("/Users/patrik/MARS/client/local/console/kotlinNetworking/images/write/apple${j}.jpg").writeBytes(buffer)
            } else {
                println("File already exists.")
                File("/Users/patrik/MARS/client/local/console/kotlinNetworking/images/write/apple${j}.jpg").writeBytes(buffer)
            }
        } catch (e: IOException) {
            println("An error occurred.")
            e.printStackTrace()
        }


    }).start()
}

private fun helpertxt(buffer: ByteArray) {
    receivetimes += System.currentTimeMillis()

    Thread(Runnable{
        val a = String(buffer)
        j+=1
        println("apple${j}.jpg is $a")
    }).start()
}
private class Start_stream_tcpjpg_receive(val ip: String, val port: Int,val stream_type: String, val client: Socket): IstreamReceive, Thread() {
    private lateinit var callback : (String)->String
    var queue = LinkedBlockingDeque<Frame>() //ByteArray
    var running = true
    //private val client = Socket(ip, port)
    private var hexread : Int = 0
    //private val input = BufferedReader(InputStreamReader(client.inputStream))
    private var hexbuffer : ByteArray = ByteArray(64)//8
    private var length : Int = 0
    private var p : Int = 0
    private var valread : Int = 0
    private var buffer : ByteArray = ByteArray(1024)
    private var t_length : Int = 0

    init {
        this.start()
    }
    override fun stopp() {
        //println("performance frames received:$i")
        i=0
        running = false
    }

    override fun getQr(): Int {
        return queue.size
    }

    fun addImgaeToQueue(buf: Frame) { //ByteArray
        queue.add(buf)
    }

    override fun getImageFromQueueu(): Frame { //ByteArray
        val a = queue.size
        //println("RECEIVED QUEUE LENGTH $a")
        return queue.takeLast()
    }
    override fun run() {
        //println("test1.receive.run started")
        //var list: ArrayList<Byte> = ArrayList<Byte>()
        do {
            var buf: ByteArray = byteArrayOf()
            //hexread = input.read(hexbuffer, 0, 8)
            hexread = client.inputStream.read(hexbuffer)
            if(hexread != 64) break
            //println(hexbuffer.size)



            val framecount = String(hexbuffer,5,16).toLong(16)
            val timestart = String(hexbuffer,22, 16).toLong(16)
            val processtime = String(hexbuffer,39, 16).toLong(16)
            length = String(hexbuffer, 56, 8).toInt(16)
            t_length = length
            p = 0
            do {
                //valread = input.read(buffer, p, length)
                var toread = buffer.size
                if (length < toread) toread = length
                valread = client.inputStream.read(buffer, 0, toread)
                if(valread < 0) break
                buf += buffer.copyOfRange(0, valread)
                length -= valread
                p += valread
                //println(valread)
            }while (length>0)
            val frame = Frame()
            frame.frameCount = framecount
            frame.timeStart = timestart
            frame.processTime = processtime
            frame.buffer = buf
            //print(buffer.joinToString(separator = "",limit = p).replace("...",""))
            //response = callback(buffer.joinToString(separator = "",limit = p).replace("...",""))
            //println("#DEBUG ControlProtocolV1: response:$response");
            //println("size of image: "+buf.size + " expected size: " + String(hexbuffer).toInt(16))
            //if (buf.size > 125) helper(buf)
            addImgaeToQueue(frame)//else helpertxt(buf)
        } while(running)
    }
}
/*
private class Start_stream_type2(ip: String, port: Int, stream_params: Array<String>): Istream, Thread() {
    var queue = LinkedBlockingDeque<ByteArray>()
    override fun send() {
        queue.addFirst(message)
    }
    override fun receive() {
        pass
    }
    override fun stopp() {

    }
    fun run() {
        while(running) {
            queue.takeLast()
        }
    }
}
*/
class StreamProtocol(ip: String, port: Int, config: String = "") {
    val stream_type: String = ""
    var stream_params: Array<String> = arrayOf<String>()
    //val stream: Start_stream_with_x
    lateinit var stream_send: IstreamSend
    lateinit var stream_receive: IstreamReceive
    init {
        /*if (config=="") {
           val stream_type: String = "type0"
           stream = Start_stream_with_x(ip, port)
        } else */if (config=="test1") {
            val socket = DatagramSocket()
            println("Start_stream_test1_send $ip $port $socket")
            stream_send = Start_stream_test1_send(ip, port, socket)
            //val socket2 = DatagramSocket(port+1, InetAddress.getByName("192.168.1.20"))
            stream_receive = Start_stream_test1_receive(stream_type, socket)
        } else if (config == "tcpjpg") {
            val socket = Socket(ip, port)
            println("tcpjpg $ip $port $socket")
            stream_send = Start_stream_tcpjpg_send(ip, port, socket)
            //val socket2 = DatagramSocket(port+1, InetAddress.getByName("192.168.1.20"))
            stream_receive = Start_stream_tcpjpg_receive(ip, port,stream_type, socket)
        }

        /*else if (config=="receive rtp h264 decode h264 inference faceblur encode h264 send rtp") {
                val stream_type: String =  "type2"
                val stream_params: Array<String> = config.split()
                stream = Start_stream_type2(ip, port, stream_params)
            }*/
    }
    fun send(n: Frame) {//ByteArray
        stream_send.send(n)
    }

    fun receive(): Frame {
        return stream_receive.getImageFromQueueu()
    }
    fun getQin():Int {
        return stream_send.getQs()
    }
    fun getQout():Int {
        return stream_receive.getQr()
    }
    fun stop() {
        stream_send.stopp()
        stream_receive.stopp()
    }
    //input params:
    //port, ip
    //stream config
    //send and receive with different types of encoder
}
