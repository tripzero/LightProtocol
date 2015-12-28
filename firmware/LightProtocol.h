/* light protocol for use with photon or arduino-like controllers*/

#include <vector>
#include <cstdint>

#if DEBUG
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
        SetAllLights = 0x06
    };
    
    LightProtocol(const T & light=T(), bool d = false)
    :debug(d), index(0), leds(light)
    {
        
    }
    
    void begin()
    {
        leds.begin();
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
  
        leds.show();
    }

    void setAllLights()
    {
        uint8_t r = getNextByte();
        uint8_t g = getNextByte();
        uint8_t b = getNextByte();

        for(uint16_t i = 0; i < leds.numLights(); i++)
        {
            leds.setPixelColor(i, r, g, b);
        }

        leds.show();
    }
    
    void clear()
    {
        for(int i=0; i<leds.numLights(); i++)
        {
            leds.setPixelColor(i, 0, 0, 0);
        }
    }
    
    void parse(const std::vector<uint8_t> & buff)
    {
        buffer = buff;
        index = 0;   
        uint8_t cmd = getNextByte();
        doCommand(cmd);
    }

    void doCommand(uint8_t cmd)
    {        
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
    }


private:
    uint16_t index;
    T leds;
    bool debug;
    std::vector<uint8_t> buffer;
};

template<class T, class N>
void processClient(T & tcpClient, std::vector<uint8_t> & buffer, N & lightProtocolParser)
{
    int msgSize = 0;
            
    while ((msgSize = tcpClient.available())) // read from client
    {
        uint8_t b = 0;
        for(int i=0; i < msgSize; i++)
        {
            b = tcpClient.read();
            if (b == '\n')
            {
                lightProtocolParser.parse(buffer);
                buffer.clear();
            }
            else if (b != -1)
            {
                buffer.push_back(b);
            }
        }
    }
}
