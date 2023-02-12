# mobile-augmented-reality-system - MARS
A demo system that uses a linux server and an android phone.

MARS is example system of testing image/video inference offloading
to a low latency server.

This project contain an Android client and Linux server application, which
support use of torch based image inference processing.

Client and the server communicate using a custom control protocol,
and data transfer is done using a custom stream protocol.
In the current (V1) demo both are based on TCP.
