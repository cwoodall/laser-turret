/*
Team Lux
Chris Woodall
Jonah Lou
Jenny Hoac
*/

//This is to compile without the X11 code
#define cimg_display 0
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

//Converts colord images to grayscale in PAL and NTSC standards
CImg<unsigned char> colorToGray(CImg<int> procImg)
{	
	
	int gryWidth = procImg.width();
	int gryHeight = procImg.height();
	int gryDepth = procImg.depth(); 
	
	unsigned char r, g, b;
	unsigned char gray = 0;
	
	CImg<unsigned char> grayImg(gryWidth, gryHeight, gryDepth, 1);
	
	for(int x = 0; x < gryWidth; x++)
	{
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

void imageProcess()
{
	//CImg<int> oriImg("image1.bmp");
	//CImg<int> currImg("image2.bmp");
	CImg<int> oriImg("image1.jpg");
	CImg<int> currImg("image2.jpg");	

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
	int avgPix = 0;
	
	grayImg1 = colorToGray(oriImg);
	grayImg2 = colorToGray(currImg);
	//grayImg1.save("gray1.bmp");
	//grayImg2.save("gray2.bmp");	
	
	for(int x = 0; x < imgWidth; x++)
		for(int y = 0; y < imgHeight; y++)	
		{
			//Finds the difference of two images and sets a threshold to find the most change in movement
			//Take avg of whole pic and then set threshold +25% abv avg
			newImage(x, y, 0, 0) = abs(grayImg2(x, y, 0, 0) - grayImg1(x, y, 0, 0));
		}
	
    //..    avgPix = 0;


    /*	for(int x = 0; x < imgWidth; x++) {
		for(int y = 0; y < imgHeight; y++)
		{
            if (newImage(x,y,0,0) > 0) {
                avgPix = (avgPix + newImage(x , y, 0, 0))/2;
            }
		}
        }*/

    //	avgPix = avgPix / (320 * 240);
    //	cout << "AVG Pix: " << avgPix << endl;
	for(int x = 0; x < imgWidth; x++) {
		for(int y = 0; y < imgHeight; y++)
		{
			newImage(x , y, 0, 0) = (newImage(x, y, 0, 0) > threshhold)? 255 : 0;
		}
    }
	
	
	//tempImg.save("test.bmp");
	newImage.save("graySub.jpg");
	
	int totWhitePixels = 0;
	int maxWhitePixels = 0;
	maxX = imgWidth / 2;
	maxY = imgHeight / 2;

	int gridX = 25;
	int gridY = 25;
	
	//This code divides the picture into grids and finds the grid with the most white
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
	//And saves as target.bmp
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
	
	
	

	//std::cout << maxX << " " << maxY;
}

int main()
{
    //    char pan[8];
    char tilt[8];
    int panCoor, tiltCoor;
	int panRange, tiltRange;
    int pwmFile;	
	char *panMin;
	char *tiltMin;
	char *panMax;
	char *tiltMax;
	
	panMin = getenv("PANMIN");
	tiltMin = getenv("TILTMIN");
	panMax = getenv("PANMAX");
	tiltMax = getenv("TILTMAX");
	
	panRange = strtol(panMax, NULL, 10) - strtol(panMin, NULL, 10);
	tiltRange = strtol(tiltMax, NULL, 10) - strtol(tiltMin, NULL, 10);
	
//	while(1)
//	{
		
		system("./uvccapture -oimage2.jpg");
		
		imageProcess();

		/*
		Some magical calculations
		to get pan and tilt
		*/
		maxX = 320 - maxX;
		
		panCoor = ((maxX*panRange)/320) + strtol(panMin, NULL, 10);
		tiltCoor = ((maxY*tiltRange)/240) + strtol(tiltMin, NULL, 10);
		
        //		sprintf(pan, "p%i\n", panCoor);

		
        //		cout << pan << "     " << tilt << endl;
//		pwmFile = fopen("/dev/pwmservo", "w");
		pwmFile = open("/dev/pwmservo", O_WRONLY | O_APPEND);
		if (pwmFile < 0)
		{
			fputs("Pwmservo module isn't loaded\n",stderr);
			return -1;
		}
		sprintf(tilt, "t%i\n", tiltCoor);
		write(pwmFile, tilt, sizeof(tilt));
        cout << tilt << "  ";
		sprintf(tilt, "p%i\n", panCoor);
        cout << tilt << endl;
		write(pwmFile, tilt, sizeof(tilt));
        cout << "max x: " << maxX << "max y: " << maxY << endl;
//		fwrite(pan, 1, sizeof(pan), pwmFile);
//		fputs(tilt, 1, sizeof(tilt), pwmFile);
		close(pwmFile);	
//		fclose(pwmFile);
//	}
	
	return 0;
}
