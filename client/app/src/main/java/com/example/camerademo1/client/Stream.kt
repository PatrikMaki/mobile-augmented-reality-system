//OLD!
//Not used in the current demo
//can be removed
import java.net.DatagramSocket
import java.net.DatagramPacket
import java.net.InetAddress

fun send_stream_with_x(port: String) { //there needs to be an ability to stopp the stream
    Thread(Runnable{
        var i = 0
        while(runningStreamGlobal){
            // here goes you code that runs in the background, this should be in multiple threads
            println("Starting thread 3")
            //Open a port to send a UDP package
            //this code is not good
            val messageStr = "abcd\n" //packets divided by "\n"
            println(messageStr)
            val socket = DatagramSocket()
            socket.broadcast = true
            println("lähetetään:")
            println(messageStr.toByteArray())
            println(messageStr.toByteArray())
            val sendData = messageStr.toByteArray()
            //println(port)
            var packet = DatagramPacket(sendData, sendData.size, InetAddress.getByName(globalIP), 4321)
            socket.send(packet)
            println("Packet Sent")
            packet = DatagramPacket(sendData, sendData.size)
            socket.receive(packet);
            val receivedBytes = packet.getData()
            val a = receivedBytes.toString(Charsets.UTF_8)
            println(receivedBytes)
            println("vastaanotetaan:")
            println(a)
            println("koira")
            //i = i+1
            Thread.sleep(1_000)
        }
        //xrunning = false
    }).start()
}

fun receive_stream_with_x() { //to be used later
    //Open a port to send a UDP package
    //this code is not good
    val messageStr = "abcd\n" //packets divided by "\n"
    val socket = DatagramSocket()
    socket.broadcast = true
    val sendData = messageStr.toByteArray()
    val sendPacket = DatagramPacket(sendData, sendData.size, InetAddress.getByName("127.0.0.1"), 8080)
    socket.send(sendPacket)
    println("Packet Sent")
}
