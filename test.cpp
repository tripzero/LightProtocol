

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

class TestClient {
public:

	TestClient(std::vector<uint8_t> frame)
	:mFrame(frame)
	{

	}

	int read()
	{
		std::cout<<"TestClient::read: ";
		if (mFrame.size())
		{
			int b = mFrame[0];
			mFrame.erase(mFrame.begin());

			std::cout<<(int)b <<std::endl;
			return b;
		}
		std::cout<<"no data"<<std::endl;
		return -1;
	}

	int available()
	{
		return mFrame.size();
	}
	
private:
	std::vector<uint8_t> mFrame;
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
	buffer.clear();

	std::vector<uint8_t> frame;
	frame.push_back(0x01); // protocol version 1:
	frame.push_back(0x08); // payload length lsb
	frame.push_back(0x00); // payload length msb

	frame.push_back(0x01); // set lights command
	frame.push_back(0x01); // num lights to set lsb
	frame.push_back(0x00); // num lights to set msb
	frame.push_back(0x01); // light id to set lsb
	frame.push_back(0x00); // light id to set msb
	frame.push_back(100); // red
	frame.push_back(100); // green
	frame.push_back(100); // blue

	{
		TestClient client(frame);

		lights.processClient(client, buffer);	
	}
	

	frame.clear();
	buffer.clear();

	/// try set all colors:
	//01 0600 06 00 00 64

	frame.push_back(0x01); // protocol version 1:
	frame.push_back(0x04); // payload length lsb
	frame.push_back(0x00); // payload length msb

	frame.push_back(0x06); // set lights command
	frame.push_back(0x00); // r
	frame.push_back(0x00); // g
	frame.push_back(0x64); // b

	{
		TestClient client(frame);
		lights.processClient(client, buffer);
	}
	
}


