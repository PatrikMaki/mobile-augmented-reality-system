# MARS Android app V1

## Directory structure

- app\src\main\java\com\example\camerademo1 -- contains Android app code
- app\src\main\assets -- contains models and labels
- app\src\main\res\layout -- contains UI layout
- app\src\main\AndroidManifest.xml -- contains permissions
- app.py - used for generating client ML models


## Installation
- Android studio: https://developer.android.com/studio 
- Python
- Android Phone with Android 10 and a camera

Testing the application is easiest by connecting your Android device
to your Android studio with USB or Wi-Fi.
You can also build an APK from Build/"Build Bundle(s) / APK(s)".
Then you can loacte the APK and upload to for exampele Google Drive
and Download it from there with the Android Device.

Currently the IP address of the server is hardcoded in:
`
mobile-augmented-reality-system\client\app\src\main\java\com\example\camerademo1\MainActivity.kt
`
The IP address needs to be same as in the server.

The app has multiple different resolution options:
- 320x240
- 640x480
- 1280x720
- 1920x1080

Of these resolutions, 640x480 is scaled correctly on the UI
and should perform well in both edge and local modes. 

## regenerating model

install dependencies for:
`
mobile-augmented-reality-system\client\app.py
`
torchvision 0.9.0

the run the Python script with Python 3