#!/bin/sh
##
# Setup Script for EC535 Project
# Authors: Chris Woodall, Jonah Lou, Jenny Hoac
# Date: April 24, 2013 
# 
# Sets up the following features:
#   - WebCam (DONE)
#   - Servo Motors (DONE)
#
# IMPORTANT !! Webcam MUST be plugged in. Takes 2 test images
#              since there have traditionally been problems with
#              the first image or two. This is for testing.
#
##
echo "Setting up WebCam"
echo "Installing webcam (V4L) kernel modules"
cd /home/root/webcam/webcam-drivers
sh install-webcam.sh
cd /home/root

echo "Adding uvccapture to path"
export PATH="$PATH:/home/root/webcam/webcam-bin"

echo "Say Cheese! Taking two still pictures"
uvccapture -o/tmp/trash-001.jpg
sleep 1
uvccapture -o/tmp/trash-002.jpg
echo "WebCam setup complete"

echo "Removing temp files"
rm /tmp/trash-*.jpg

echo "Installing Servo Motor kernel module (pwmservo)"
mknod /dev/pwmservo c 61 0

insmod $HOME/pwmservo.ko


echo "Setup Complete. Thank you!"

