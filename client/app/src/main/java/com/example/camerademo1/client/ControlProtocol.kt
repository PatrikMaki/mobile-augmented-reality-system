import java.net.Socket
import java.io.BufferedReader
import java.io.InputStreamReader

/*
Class, which communicates with the server's control protocol.
 */
class ControlProtocol(server: String, port: Int){
    private lateinit var callback : (String)->String
    private val client = Socket(server, port)
    private val input = BufferedReader(InputStreamReader(client.inputStream))
    private var buffer : CharArray = CharArray(1024)
    private var hexbuffer : CharArray = CharArray(4)
    private var hexread : Int = 0
    private var new_socket : Int = 0
    private var t_length : Int = 0
    private var length : Int = 0
    private var p : Int = 0
    private var valread : Int = 0
    private var response : String = ""

    /*
    Control Protocols Receive loop:
    - receives response messages from server
    - forwards the messages to the main code
     */
    fun rec() { //receive loop, called from client
        do {
            hexread = input.read(hexbuffer, 0, 4)
            if(hexread != 4) break
            length = hexbuffer.joinToString(separator = "", limit = 4).toInt(16)
            t_length = length
            p = 0
            do {
                valread = input.read(buffer, p, length)
                if(valread < 0) break
                length -= valread
                p += valread

            }while (length>0)
            response = callback(buffer.joinToString(separator = "",limit = p).replace("...",""))
        } while(runningControlGlobal)
    }
    fun setCallBack(callback: (String)->String) {
        this.callback = callback
    }

    // Functions for sending messages to server:
    fun hello() { client.outputStream.write("0005Hello".toByteArray()) }
    fun test() { client.outputStream.write("000Ftest connection".toByteArray()) }
    //fun startx() { client.outputStream.write("001Dstart stream, with protocol x".toByteArray()) }
    //fun starttest() { client.outputStream.write("0017start stream with test1".toByteArray()) }
    fun start(config: String) {
        if (config == "") {
            client.outputStream.write("001Dstart stream, with protocol x".toByteArray())
        } else if (config == "test1") {
            client.outputStream.write("0017start stream with test1".toByteArray())
        } else if (config == "tcpjpg") {
            client.outputStream.write("0006tcpjpg".toByteArray())
        } else if (config == "tcpjpg classify") {
            client.outputStream.write("0008tcpjpg c".toByteArray())
        }
    }
    fun stopp() {
        client.outputStream.write("000Bstop stream".toByteArray())
        runningStreamGlobal = false
        //stopp stream and stopp stream thread
    }
    fun close() {
        client.outputStream.write("0010close connection".toByteArray())
        runningControlGlobal = false
        //stopp control protocol thread
    }
}