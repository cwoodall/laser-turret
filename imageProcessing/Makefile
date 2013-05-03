## 
# imageProcessing Makefile - EC535 Project
# Christopher Woodall <cwoodall at bu.edu>
# Jonah Lou <jonahlou@bu.edu>
# Jenny Hoac <jwfhoac@bu.edu>
# Date: Fri May  3 16:46:24 EDT 2013
#
# Setup for 
##

## Uncomment for setup in personal directory (Chris W.)
#CC = $(HOME)/bin/arm-linux/bin/arm-linux-g++ 

## Comment and change CC if base compiler is broken
CC = arm-linux-g++

## Flags for linking libjpeg and stdc++ to make sure it runs on gumstix.
C_FLAGS = -Wall -L. -Wl,-Bstatic -ljpeg libjpeg.a -Wl,-Bstatic -lstdc++ -static -Os
TARGET = imgProcessing

imgProcessing: $(TARGET).cpp
	$(CC) $(TARGET).cpp $(C_FLAGS)  -o $(TARGET)

clean:
	rm imgProcessing