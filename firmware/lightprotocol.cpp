#include "lightprotocol.h"

void processClient(const TcpClient & client, std::vector<uint8_t> & buffer, const LightProtocolParser & lightProtocolParser)
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