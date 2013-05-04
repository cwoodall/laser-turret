CC=arm-linux-gcc
CPP=arm-linux-g++
APP_BINARY=uvccapture
VERSION = 0.4
PREFIX=$HOME/uvc_bin

WARNINGS = -Wall


CFLAGS = -std=gnu99 -O2 -DLINUX -DVERSION=\"$(VERSION)\" $(WARNINGS)  -L${HOME}/src/jpeg-8d/lib -I${HOME}/src/jpeg-8d/include -isystem ${HOME}/src/jpeg-8d/include
CPPFLAGS = $(CFLAGS)

OBJECTS= uvccapture.o v4l2uvc.o


all:    uvccapture

clean:
    @echo "Cleaning up directory."
    rm -f *.a *.o $(APP_BINARY) core *~ log errlog

install:
    install $(APP_BINARY) $(PREFIX)

# Applications:                                                                                                                                                                   
uvccapture: $(OBJECTS)
    $(CC)  $(OBJECTS) $(XPM_LIB) $(MATH_LIB) -L ${HOME}/src/jpeg-8d/lib -I ${HOME}/src/jpeg-8d/include -ljpeg -o $(APP_BINARY)
