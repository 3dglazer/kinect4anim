#ifndef RGBWRITER_H
#define RGBWRITER_H
#include "RGBData.h"
#include <stdio.h>
#include <string>
#define MILIS2SECONDS 1.0/1000.0

//jpeg writing capability
#include "writeJPEG.h"

using namespace std;
//consumer thread which takes care of color info to jpg dumping
class RGBWriter : public Thread {
public:
	RGBWriter(CQueue<RGBData*> &queue,string &filePath,int &jpegLib_Width,int &jpegLib_Height,int quality) : imageQueue(queue),fpth(filePath),jpgLib_imWidth(jpegLib_Width),jpgLib_imHeight(jpegLib_Height),m_quality(quality) {};
	void* run() {
		// Remove 1 item and process it. Blocks if no items are 
        // available to process.
		RGBData* item;
        for (int i = 0;; i++) {
			if (imageQueue.PopElement(item)) {
				char *fn = new char[fpth.size() + 50];
				sprintf(fn, "%skinectDump-%f.%s", &fpth[0], (float)item->getTimeStamp()*MILIS2SECONDS, "jpg");
				writeJPG(fn, item->getData(), item->getWidth(), item->getHeight(), m_quality);
				delete item;
				delete[] fn;
			}else {
				usleep(30000); //just don't stress the processor if not needed
			}
		}
		printf("What happened the writer has quit!\n");
		return NULL;
	};
private:
	CQueue<RGBData*> &imageQueue;
	string fpth;// file path
	int jpgLib_imWidth;
	int jpgLib_imHeight;
	int m_quality;
	
	void writeJPG(char fileName[],uint8_t* pixels,int width,int height,int quality){
		if (pixels==NULL) {
			return;
		}
		jpgLib_imWidth=width;
		jpgLib_imHeight=height;
		set_JPEG_data(pixels);
		write_JPEG_file (fileName, quality);
	}
};
#endif