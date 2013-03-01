#ifndef RGBDATA_H
#define RGBDATA_H
// basicaly an image container
class RGBData {
public:
	RGBData(uint8_t* rgbColor,long tmStamp,int width, int height):rgb(rgbColor),timeStamp(tmStamp),m_width(width),m_height(height){};
	~RGBData(){
		//printf("deleted rgbdata should free memory");
		delete[] rgb;
	};
	uint8_t* getData(){
		return rgb;
	};
	long getTimeStamp(){return timeStamp;};
	int getWidth(){return this->m_width;};
	int getHeight(){return this->m_height;};
private:
	uint8_t* rgb;
	long timeStamp;
	int m_width,m_height;
};
#endif