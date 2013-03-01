#ifndef DEPTHWRITER_H
#define DEPTHWRITER_H
#include "DepthData.h"
#include <stdio.h>
#include <string>

//OpneEXR
#include <exception>
#include <ImfInputFile.h>
#include <ImfRgbaFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <half.h>
#include <ImfOutputFile.h>
#include <ImfStringAttribute.h>
#include <ImfArray.h>
using namespace Imf;
using namespace Imath;

class DepthWriter : public Thread {
public:
	~DepthWriter(){delete[] rgbaPixels;};
	DepthWriter(CQueue<DepthData*> &queue,string &filePath,int width,int height,Compression comp) : imageQueue(queue),fpth(filePath),m_width(width),m_height(height),m_comp(comp) {
		rgbaPixels=new Rgba[width*height];
	};
	void* run() {
		// Remove 1 item and process it. Blocks if no items are 
        // available to process.
		DepthData* item;
        for (int i = 0;; i++) {
			if (imageQueue.PopElement(item)) {
				char *fn = new char[fpth.size() + 50];
				sprintf(fn, "%skinectDump-%f.%s", &fpth[0], (float)item->getTimeStamp()*MILIS2SECONDS, "exr");
				writeEXR(fn, item->getData(), m_width, m_height, m_comp);
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
	CQueue<DepthData*> &imageQueue;
	string fpth;// file path
	int m_width;
	int m_height;
	Compression	m_comp;
	Rgba* rgbaPixels;
	//Depth data are written into red channel of exr image.
	void writeEXR(const char fileName[],const half* gPixels,int width,int height, Compression comp){
		if (gPixels==NULL) {
			return;
		}		
		Header header (width, height); // 1
		header.channels().insert ("R", Channel (HALF)); // 2
		header.compression() = comp;
		OutputFile file (fileName, header); // 4
		FrameBuffer frameBuffer; // 5
		frameBuffer.insert ("R", // name // 6
							Slice (HALF, // type // 7
								   (char *) gPixels, // base // 8
								   sizeof (*gPixels) * 1, // xStride// 9
								   sizeof (*gPixels) * width)); // yStride// 10
		file.setFrameBuffer (frameBuffer); // 16
		file.writePixels (height); // 17
	}
};
#endif