# Laser Turret
## Compilation Instructions
As a note uvccapture, the V4L drivers and libjpeg are all given compiled for the gumstix boards and OS we were given.

### lib/uvccapture

Assumes jpeg-8b installed to $HOME/src/jpeg-8d. Rewrote makefile to make cross-compilation possible

> make


### V4L drivers (lib/uvcvideo.tar.bz2)

Worked out of the box. First need to untar


> tar -xf uvcvideo.tar.bz2  
> cd uvcvideo  
> make


### jpeg-8d (lib/jpeg-8d)

> $ ./configure --host=arm-linux  --prefix=$HOME/src/jpeg-8d
> $ make  
> $ make install  


### pwmservo
Standard kernel make file

> $ make


### imgProcessing

Makefile was created to set CFLAGS, needed to static link against libstdc++ because it didn't seem to exist in the right version on the gumstix board.

> $ make


## Directory Structure for gumstix

    /-- home
        +-- root
            |-- ethernet_setup.sh: setup script for ethernet [in scripts].
            |-- setup.sh: setup script for webcam and pwmservo.ko [in scripts].
            |-- imageProcessing: image processing binary [source in imageProcessing].
            |-- pwmservo.ko: PWM servo motor driver kernel module [source in pwmservo].
            |-- webcam: compiled kernel drivers for gumstix (V4L) and 
            |   |       compiled uvccapture [inside of binaries-and-uvccapture.tar.gz]
            |   |-- webcam-drivers :
            |   |   |-- compat_ioctl32.ko
            |   |   |-- uvcvideo.ko
            |   |   |-- v4l2-common.ko
            |   |   |-- videodev.ko
            |   |   +-- v4l1-compat.ko
            |   +-- webcam-bin
            |       +-- uvccapture
            +-- uvccapture: link to uvccaptur in ~/webcam/webcam-bin
