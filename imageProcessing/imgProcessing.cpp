/*
Team Lux
Chris Woodall
Jonah Lou
Jenny Hoac
*/

//This is to compile without the X11 code
#define cimg_display 0
//This is to compile with jpegs. You have to link libjpg
#define cimg_use_jpeg

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include "CImg.h"
#include <fcntl.h>
using namespace cimg_library;
using namespace std;

//Coordinates to be returned
static int maxX;
static int maxY; 

//Converts colord images to grayscale using a formula derived from PAL and NTSC standards
//You pass in a CImg<int> and it returns a CImg<unsigned char>
CImg<unsigned char> colorToGray(CImg<int> procImg)
{	
	//Getting the dimensions of the images taken
	int gryWidth = procImg.width();
	int gryHeight = procImg.height();
	int gryDepth = procImg.depth(); 
	
	//variables needed for changing from color to grayscale
	unsigned char r, g, b;
	unsigned char gray = 0;
	
	
	
	CImg<unsigned char> grayImg(gryWidth, gryHeight, gryDepth, 1);
	
	for(int x = 0; x < gryWidth; x++)
	{
		//Finds the RGB values of each pixel and then apply a *MODDED*
		//calculation. The PAL / NTSC standard is commented out. 
		//We used to a formula with less emphasis on red to taken into accout
		//the red laser dot. 
		for(int y = 0; y < gryHeight; y++)
		{
			r = procImg(x, y, 0, 0);
			g = procImg(x, y, 0, 1);
			b = procImg(x, y, 0, 2);
			//PAL and NTSC
			//Y = 0.299*R + 0.587*G + 0.114*B
			gray = round(0.114*((int)r) + 0.587*((int)g) + 0.299*((int)b));
			
			grayImg(x, y, 0, 0) = gray;
		}
	}
	return grayImg;
}

//Does the object / motion detection algorithm
void imageProcess()
{
	//Variables for finding the location of the target
	int totWhitePixels = 0;
	int maxWhitePixels = 0;
	maxX = imgWidth / 2;
	maxY = imgHeight / 2;

	int gridX = 25;
	int gridY = 25;
	
	CImg<int> oriImg("image1.jpg");
	CImg<int> currImg("image2.jpg");	
	
	//Used for export commands so we can control variables from outside
	char *thresh;	
	int threshhold;
	int imgWidth = oriImg.width();
	int imgHeight = oriImg.height();
	int imgDepth = oriImg.depth();
	
	thresh = getenv("THRESHHOLD");
	threshhold = strtol(thresh, NULL, 10);
	
	CImg<unsigned char> grayImg1(imgWidth, imgHeight, imgDepth, 1);
	CImg<unsigned char> grayImg2(imgWidth, imgHeight, imgDepth, 1);
	CImg<unsigned char> newImage(imgWidth, imgHeight, imgDepth, 1);
	
	grayImg1 = colorToGray(oriImg);
	grayImg2 = colorToGray(currImg);
	//Uncomment to see image1 and image2 in grayscale
	//grayImg1.save("gray1.jpg");
	//grayImg2.save("gray2.jpg");	
	
	for(int x = 0; x < imgWidth; x++)
	{
		for(int y = 0; y < imgHeight; y++)	
		{
			//Finds the difference of two images
			newImage(x, y, 0, 0) = abs(grayImg2(x, y, 0, 0) - grayImg1(x, y, 0, 0));
		}
	}
    
	for(int x = 0; x < imgWidth; x++) 
	{
		for(int y = 0; y < imgHeight; y++)
		{
			//Sets a threshold for the imagea and make it only black and white
			newImage(x , y, 0, 0) = (newImage(x, y, 0, 0) > threshhold)? 255 : 0;
		}
    	}
	
	newImage.save("graySub.jpg");
	
	
	//Creates a grid of gridX by gridY and sweeps the entire picture while
	//keeping track of the x, y, and totWhitePixels of the grid with the
	//highest amount of white (aka contrast from the base image)
	for(int x = 0; x < imgWidth - gridX; x += gridX) 
		for(int y = 0; y < imgHeight - gridY; y += gridY) 
		{
			for(int xx = x; xx < x + gridX; xx++)
				for(int yy = y; yy < y + gridY; yy++)
					if(newImage(xx, yy, 0, 0) == 255)
						totWhitePixels++;	
			if(totWhitePixels > maxWhitePixels)
			{
				maxX = x + (gridX / 2);
				maxY = y + (gridY / 2);
				maxWhitePixels = totWhitePixels;
				totWhitePixels = 0;
			}
		}
	
	//Draws a red '+' just for debugging purposes
	//And saves as target.jpg
	for(int x = maxX - 5; x < maxX + 5; x++)
	{
		currImg(x, maxY, 0, 0) = 255;
		currImg(x, maxY, 0, 1) = 0;
		currImg(x, maxY, 0, 2) = 0;
	}
	for(int y = maxY - 5; y < maxY + 5; y++)
	{
		currImg(maxX, y, 0, 0) = 255;
		currImg(maxX, y, 0, 1) = 0;
		currImg(maxX, y, 0, 2) = 0;
	}	
	
	currImg.save("target.jpg");
}

int main()
{
	//Varibles for obtaining and calculating the pan / tilt
 	char tilt[8];
   	int panCoor, tiltCoor;
	int panRange, tiltRange;
    	int pwmFile;	
	char *panMin;
	char *tiltMin;
	char *panMax;
	char *tiltMax;
	
	//Parameters that can be set outside of this code for calibration
	panMin = getenv("PANMIN");
	tiltMin = getenv("TILTMIN");
	panMax = getenv("PANMAX");
	tiltMax = getenv("TILTMAX");
	
	panRange = strtol(panMax, NULL, 10) - strtol(panMin, NULL, 10);
	tiltRange = strtol(tiltMax, NULL, 10) - strtol(tiltMin, NULL, 10);

	//Calls the image capture binary
	system("./uvccapture -oimage2.jpg");
	
	imageProcess();
	//Reverse the pixel for calculation purposes (image was reversed)
	maxX = 320 - maxX;
	
	//320 is imgWidth and 240 is imgHeight
	panCoor = ((maxX*panRange)/320) + strtol(panMin, NULL, 10);
	tiltCoor = ((maxY*tiltRange)/240) + strtol(tiltMin, NULL, 10);
	
	pwmFile = open("/dev/pwmservo", O_WRONLY | O_APPEND);
	if (pwmFile < 0)
	{
		fputs("Pwmservo module isn't loaded\n",stderr);
		return -1;
	}
	
	//Sending command to servo module and print for debugging purposes
	sprintf(tilt, "t%i\n", tiltCoor);
	write(pwmFile, tilt, sizeof(tilt));
	cout << tilt;
	sprintf(tilt, "p%i\n", panCoor);
	cout << tilt << endl;
	write(pwmFile, tilt, sizeof(tilt));
	cout << "max x: " << maxX << "max y: " << maxY << endl;
	close(pwmFile);	

	
	return 0;
}
