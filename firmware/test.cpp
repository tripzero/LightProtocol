

#include "LightProtocol.h"
#include <iostream>
#include <vector>

class TestLights
{
public:
	TestLights()
	:mNumLights(0) 
	{

	}

	void begin()
	{
		std::cout<<"begin()"<<std::endl;
	}

	void setPixelColor(uint16_t id, uint8_t r, uint8_t g, uint8_t b)
	{
		std::cout<<"setPixelColor() "<<id<< " "<< (int)r << " " << (int)g << " " << (int)b <<std::endl;
	}

	uint16_t numLights()
	{
		return mNumLights;
	}

	void updateLength(uint16_t l)
	{
		std::cout<<"updateLength(): "<< l << std::endl;
		mNumLights = l;
	}

	void show()
	{
		std::cout<<"show()"<<std::endl;
	}


private:
	uint16_t mNumLights;
};


int main(int argc, char** argv)
{
	std::cout<<"hello world"<< std::endl;
	LightProtocol<TestLights> lights;
	std::vector<uint8_t> buffer;

	/// test set num lights:

	buffer.push_back(0x02);
	buffer.push_back(0x06);
	buffer.push_back(0x00);

	lights.parse(buffer);

	buffer.clear();

	/// test set single light color:

	buffer.push_back(0x01);
	buffer.push_back(0x01);
	buffer.push_back(0x00);
	buffer.push_back(0x00);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);

	lights.parse(buffer);

	buffer.clear();

	// test set multiple colors:

	//header:
	buffer.push_back(0x01);
	buffer.push_back(0x06);
	buffer.push_back(0x00);

	//light 1:
	buffer.push_back(0x00);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);

	//light 2:
	buffer.push_back(0x01);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);
	
	//light 3:
	buffer.push_back(0x02);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);

	//light 4:
	buffer.push_back(0x03);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);

	//light 5:
	buffer.push_back(0x04);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);

	//light 6:
	buffer.push_back(0x05);
	buffer.push_back(0x00);
	buffer.push_back(255);
	buffer.push_back(255);
	buffer.push_back(255);

	lights.parse(buffer);

	buffer.clear();

	///Test set all lights:

	buffer.push_back(0x06);
	buffer.push_back(128);
	buffer.push_back(128);
	buffer.push_back(128);

	lights.parse(buffer);
}


