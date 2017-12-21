#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QByteArray>

#include <iostream>
#include <vector>

#include "LightProtocol.h"

class TestLights
{
public:
    TestLights()
    :mNumLights(10)
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

class TestClient
{
public:
    TestClient(QTcpSocket* socket)
    :mSocket(socket)
    {
    }

    int read()
    {
        auto buff = mSocket->read(1);
        if (!buff.count())
        {
            std::cout<<"nothing to read..."<<std::endl;
            return -1;
        }

        return (int)buff[0];
    }

    int available()
    {
        auto available = mSocket->bytesAvailable();

        return available;
    }

    QTcpSocket* socket()
    {
        return mSocket;
    }

private:
    QTcpSocket* mSocket;
};

class TestUdpClient
{
public:
    TestUdpClient(QUdpSocket* socket)
    :mSocket(socket)
    {
    }

    int read()
    {
        if(!buffer.count())
        {
            buffer = mSocket->receiveDatagram().data();
            if (!buffer.count())
            {
                std::cout<<"nothing to read..."<<std::endl;
                return -1;
            }
        }

        if(buffer.count())
        {
            int val = (int)buffer[0];
            buffer.remove(0, 1);

            return val;
        }
    }

    int available()
    {
        if(buffer.count()) {
            debugOut("buffer count:");
            debugOut(buffer.count());
            return buffer.count();
        }

        auto available = mSocket->pendingDatagramSize();

        return available;
    }

    QUdpSocket* socket()
    {
        return mSocket;
    }

private:
    QUdpSocket* mSocket;
    QByteArray buffer;
};


template<class T>
void testTcp(std::vector<uint8_t> buffer, LightProtocol<T> lights)
{
    TestClient client(nullptr);
    QTcpServer server;
    server.listen(QHostAddress::Any, 1888);
    std::cout<<"Listening..."<<std::endl;


    while(true)
    {
        if (client.socket() && client.socket()->state() == QAbstractSocket::ConnectedState)
        {
            client.socket()->waitForReadyRead(1000);
            lights.processClient(client, buffer);
        }

        else
        {
            server.waitForNewConnection(1000);
            if (server.hasPendingConnections())
            {
                std::cout<<"We have a connection!"<<std::endl;
                auto sock = server.nextPendingConnection();
                client = TestClient(sock);
            }
        }

    }
}

template<class T>
void testUdp(std::vector<uint8_t> buffer, LightProtocol<T> lights)
{
    QUdpSocket socket;
    socket.bind(QHostAddress::LocalHost, 1888);

    TestUdpClient client(&socket);

    while(true)
    {
        if(socket.hasPendingDatagrams())
        {
            std::cout<<"We have a connection!"<<std::endl;
            socket.waitForReadyRead(1000);
            lights.processClient(client, buffer);
        }
    }

}

int main(int argc, char** argv)
{
    std::vector<uint8_t> buffer;
    LightProtocol<TestLights> lights;

    //testTcp(buffer, lights);
    testUdp(buffer, lights);

    return 1;
}
