#include <QTcpServer>
#include <QTcpSocket>
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

int main(int argc, char** argv)
{
	QTcpServer server;
	TestClient client(nullptr);
	std::vector<uint8_t> buffer;
	LightProtocol<TestLights> lights;

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

	return 1;
}
