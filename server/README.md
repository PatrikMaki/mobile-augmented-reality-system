# MARS Server
    
These folders contains Linux version of the server side code for MARS Project.
The MARS Server is an Augmented Reality data processing server framework.
The server contains a control protocol and chained and threaded data frame stream processors.
The stream threads can be configured with control protocol potentially to support dynamic
selection of communication protocols, message transcoding, and inference processing.
    
## Directory structure

- server -- this directory
- server/CMakeList.txt -- the cmake file definition file
- server/control -- the control protocol
- server/stream  -- contains the stream protocol initializer
- server/stream/threads -- contains the 5 chained threaded handlers for: receive, decode, inference, encode, and send
- server/stream/threads/implementations -- contains demo and torch image model inference code implementations
- server/stream/threads/implementations/test0 - a demo UDP packet test
- server/stream/threads/implementations/test1 - a demo UDP controller cuda graph problem executor which takes size of the graph in text as a parameter and returns some sum of the graph data back.
- server/stream/threads/implementations/tcp_jpg - a JPEG over TCP Stream image stream copier which just takes frames and returns those back to provide network latency measurement
- server/stream/threads/implementations/pytorch - a JPEG over TCP Stream image classifier using Pytorch based video inference model using Cuda with C++ torch native API and mobilenet-v3 model to classify the images and return back the labeled imaged.
- server/test/testclient.cpp - contains a simple test client to test the TCP based image processing

## How to build    
```
mkdir build
cd build
cmake ..
make
./mars
```

## Future ideas

- Add configurations to the control protocol messages
- Add session handling with login
- Use HTTPS/WebSockets for Control protocol
- Implement RTP/WebRTC Video streaming with H.264/H.265
