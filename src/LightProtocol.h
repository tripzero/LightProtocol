/* light protocol for use with photon or arduino-like controllers*/

#ifndef _LIGHTPROTOCOL_H_
#define _LIGHTPROTOCOL_H_

#include <cstdint>

#ifdef DEBUG_ME
#include <string>
#include <iostream>

#define String std::string

void debugOut(const auto & msg)
{
	std::cout<<"debug: "<< msg << std::endl;
}

#else

#define debugOut(x)

#endif

template <class T>
T min(const T & one, const T & two)
{
	if (one < two)
		return one;

	return two;
}

class ByteArray {
public:

	ByteArray(uint16_t max_size = 1024)
	:capacity(0), max_size(max_size), buffer(new uint8_t[max_size])
	{

	}

	ByteArray &operator=(const ByteArray &other)
	{
		capacity = other.capacity;
		buffer = other.buffer;

		return *this;
	}

	uint8_t operator[](int index)
	{
		if (index >= capacity)
		{
			return -1;
		}

		return buffer[index];
	}

	void push_back(uint8_t b)
	{
		if (capacity < max_size)
			buffer[capacity++] = b;
	}

	int size() const 
	{
		return capacity;
	}

	void clear()
	{
		capacity = 0;
	}

	int capacity;
	uint16_t max_size;
	uint8_t *buffer;
};


template <class T>
class LightProtocol
{
public:
	enum Command {
		None = 0x00,
		SetLights = 0x01,
		SetNumLights = 0x02,
		Clear = 0x03,
		LightThreshold = 0x04,
		Debug = 0x05,
		SetAllLights = 0x06,
		SetSeries = 0x07
	};

	LightProtocol(const T & light=T(), bool d = false, int * last_cmd = nullptr)
	:debug(d), index(0), leds(light), msgLength(0), supportedVersion(0x01), last_cmd(last_cmd)
	{
		debugOut("LightProtocol instantiated");
	}

	void begin()
	{
		leds.begin();
	}

	virtual void show()
	{
		leds.show();
	}

	uint8_t getNextByte()
	{
		if(index+1 > buffer.size())
		{
			return 0;
		}

		return buffer[index++];
	}

	void setLights(uint16_t numLights)
	{
		debugOut("Trying to change some lights: ");
		debugOut(numLights);
		uint8_t lowbit;
		uint16_t id;
		uint8_t r;
		uint8_t g;
		uint8_t b;

		for(uint16_t i = 0; i < numLights; i++)
		{
			lowbit = getNextByte();
			id = (getNextByte() << 8) | (lowbit);
			r = getNextByte();
			g = getNextByte();
			b = getNextByte();
			leds.setPixelColor(id, r, g, b);
		}
	}

	void setAllLights()
	{
		debugOut("setAllLights:");

		uint8_t r = getNextByte();
		uint8_t g = getNextByte();
		uint8_t b = getNextByte();

		for(uint16_t i = 0; i < leds.numLights(); i++)
		{
			leds.setPixelColor(i, r, g, b);
		}
	}

	void setSeries()
	{
		uint16_t startId;
		uint16_t length;
		uint8_t lsb;

		lsb = getNextByte();
		startId = (getNextByte() << 8) | lsb;
		lsb = getNextByte();
		length = (getNextByte() << 8) | lsb;

		uint8_t r = getNextByte();
		uint8_t g = getNextByte();
		uint8_t b = getNextByte();

		for(uint16_t i = startId; i < length; i++)
		{
			leds.setPixelColor(i, r, g, b);
		}
	}

	void clear()
	{
		debugOut("clear()");
		for(int i=0; i<leds.numLights(); i++)
		{
			leds.setPixelColor(i, 0, 0, 0);
		}
	}

	void parse(const ByteArray & buff)
	{
		buffer = buff;
		index = 0;
		while (index < buffer.size())
		{
			uint8_t cmd = getNextByte();
			doCommand(cmd);
		}
	}

	void doCommand(uint8_t cmd)
	{
		if (last_cmd != nullptr)
			*last_cmd = cmd;

		if(cmd == SetLights)
		{
			/**
			* SetLights
			* [cmd 8][numlights 16][lightId 16][R 8][G 8][B 8]...
			**/
			debugOut("cmd: SetLights");
			uint8_t numlightsLowbit = getNextByte();
			uint16_t numLights = (getNextByte() << 8) | (numlightsLowbit);

			debugOut("num lights to set" );
			debugOut(numLights);

			if(numLights > leds.numLights())
			{
				return;
			}

			setLights(numLights);
		}
		else if(cmd == SetNumLights)
		{
			/**
			* SetNumLights
			* [0x02 8bits][numlights 16bits]
			**/
			debugOut("cmd: SetNumLights");

			uint8_t numlightsLowbit = getNextByte();

			debugOut("lsb: ");
			debugOut(numlightsLowbit);

			uint16_t numLights = getNextByte() << 8;
			numLights |= (numlightsLowbit);

			//debugOut("setting number of lights to " + String(numLights));

			leds.updateLength(numLights);
		}
		else if(cmd == Clear)
		{
			/**
			* Clear
			* [0x03 8bits]
			**/
			clear();
		}
		else if(cmd == Debug)
		{
			uint8_t d = getNextByte();
			debug = d != 0;
		}
		else if(cmd == SetAllLights)
		{
			setAllLights();
		}
		else if(cmd == SetSeries)
		{
			setSeries();
		}
	}

	template<class C>
	void processClient(C & client, ByteArray & buffer)
	{
		uint16_t msgSize = client.available();

		debugOut("msgSize: ");
		debugOut(msgSize);

		if (msgSize < 3)
		{
			debugOut("msg size too small");
			return;
		}

		if (msgLength == 0)
		{
			uint8_t version = client.read();

			if (version != supportedVersion)
			{
				/// we only support version 1
				debugOut("we only support protocol version:");
				debugOut(supportedVersion);
				debugOut("received version value:");
				debugOut(version);

				return;
			}

			uint8_t b = client.read();
			msgLength = b | (client.read() << 8);

			debugOut("payload length: ");
			debugOut(msgLength);

		}

		for(int i=0; i < min(msgLength, msgSize); i++)
		{
			uint8_t b = client.read();
			buffer.push_back(b);

			if (buffer.size() == msgLength)
			{
				parse(buffer);
				buffer.clear();
				msgLength = 0;
			}
		}
	}


private:
	uint16_t index;
	T leds;
	bool debug;
	ByteArray buffer;
	uint16_t msgLength;
	uint8_t supportedVersion;
	int * last_cmd;
};

#endif
