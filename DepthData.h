#ifndef DEPTHDATA_H
#define DEPTHDATA_H
//depth data container
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

class DepthData {
public:
	DepthData(half* depthmm,long tmStamp):depthData(depthmm),timeStamp(tmStamp){};
	~DepthData(){delete[] depthData;};
	half* getData(){
		return depthData;
	};
	long getTimeStamp(){return timeStamp;};
private:
	half* depthData;
	long timeStamp;
};
#endif

