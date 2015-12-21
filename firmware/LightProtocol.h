/* light protocol for use with photon or arduino-like controllers*/

#include <vector>

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
        Debug = 0x05
    };
    
    LightProtocol(const T & light=T(), bool d = false)
    :debug(d), index(0), leds(light)
    {
        
    }
    
    void begin()
    {
        debugOut("We've only just begun...");
        leds.begin();
    }
    
    uint8_t getNextByte()
    {
        if(index+1 > buffer.size())
        {
            debugOut("requested a byte more than we have...");
            return 0;
        }
        
        return buffer[index++];
    }

    void setLights(uint16_t numLights)
    {
        //debugOut("Trying to change some lights: ");
        //debugOut(String(numLights));
        
        for(uint16_t i = 0; i < numLights; i++)
        {
            uint8_t lowbit = getNextByte();
            uint16_t id = (getNextByte() << 8) | (lowbit);
            uint8_t r = getNextByte();
            uint8_t g = getNextByte();
            uint8_t b = getNextByte();
            //debugOut("Setting light(" + String(id) + ") to: " + String(r) + ", " + String(g) + ", " + String(b));
            leds.setPixelColor(id, r, g, b);
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
        debugOut("cmd: " + String(cmd));
        
        if(cmd == SetLights)
        {
            /**
            * SetLights
            * [cmd 8][numlights 16][lightId 16][R 8][G 8][B 8]...
            **/
            //debugOut("cmd: SetLights");
            uint8_t numlightsLowbit = getNextByte();
            uint16_t numLights = (getNextByte() << 8) | (numlightsLowbit & 0xff);
            
            //debugOut("num lights to set" + String(numLights));
            
            if(numLights > leds.numLights())
            {
                debugOut("number of lights to set is higher than the number of lights I have");
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
            //debugOut("cmd: SetNumLights");
            
            uint8_t numlightsLowbit = getNextByte();
            
            //debugOut("lsb: " + String(numlightsLowbit));
            
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
            debugOut("cmd: Clear");
            clear();
        }
        else if(cmd == Debug)
        {
            debugOut("cmd: Debug");
            uint8_t d = getNextByte();
            debug = d != 0;
        }
        /*else if(cmd == LightThreshold)
        {
            lightThreshold = getNextByte();
        }*/
    }
    
    void debugOut(String msg)
    {
        if(debug)
            Particle.publish("debug", msg, PRIVATE);
    }
    
private:
    uint index;
    T leds;
    TCPClient client;
    bool debug;
    std::vector<uint8_t> buffer;
};

template<class T>
void processClient(const T & tcpClient, std::vector<uint8_t> & buffer, const LightProtocolParser & lightProtocolParser)
{
    int msgSize = 0;
            
    while ((msgSize = client.available())) // read from client
    {
        lightProtocolParser.debugOut("msg size: " + String(msgSize));
        uint8_t b = 0;
        for(int i=0; i < msgSize; i++)
        {
            b = client.read();
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
