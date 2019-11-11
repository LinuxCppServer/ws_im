#ifndef __PACK__HH
#define __PACK__HH
#include <vector>
#include <string>
#include <iterator>
#include <iostream>

struct CPack
{
	std::vector<unsigned char> mBuff;
	CPack(const char * data, const size_t & wlen)
	{
		if (data)
		{
			for (size_t i = 0; i < wlen; i++)
			{
				mBuff.push_back(data[i]);
			}
		}
	}
	CPack(const std::string &data)
	{
		if (!data.empty())
		{
			for (size_t i = 0; i < data.size(); i++)
			{
				mBuff.push_back(data[i]);
			}
		}
	}
	~CPack(){}
	void show()const
	{
		std::cout << "\nlen: " << size() << " [" << std::endl;
		for (size_t i = 0; i < size(); i++)
		{
			if ((i + 1) % 20 == 0)
			{
				std::cout << std::endl;
			}
			if (mBuff[i] == '\0')
			{
				std::cout << "0x00" << " ";
				continue;
			}
			std::cout << std::hex << (unsigned char)(mBuff[i]) << " ";
		}
		std::cout << "\n]" << std::endl;
	}

	size_t size()const { return mBuff.size(); }
	
	char* info()const
	{
		char *data = new char[mBuff.size()];
		std::vector<unsigned char>::const_iterator start = mBuff.begin();
		for (std::vector<unsigned char>::const_iterator it = start; it != mBuff.end(); it++)
		{
			data[it - start] = *it;
		}
		return data;
	}

	friend std::ostream& operator<< (std::ostream& out, const CPack& pack)
	{
		pack.show();
		return out;
	}

	static CPack mkWebSocketHandShakeReq(const char *ip, int port)
	{
		/*
		GET / HTTP / 1.1
		Host: ip:port
		Origin : http ://127.0.0.1:3000
		Connection : Upgrade
		Upgrade : websocket
		Sec - WebSocket - Version : 13
		Sec - WebSocket - Key : w4v7O6xFTi36lq3RNcgctw ==
		*/
		char line[1024] = { '\0' };
		sprintf(line, "GET http://%s:%d HTTP/1.1\r\n", ip, port);
		std::string req = line;
		req += "Connection: keep-alive\r\n";
		req += "Upgrade: websocket\r\n";
		req += "\r\n";

		std::cout << "@ws_handshake req: " << std::endl;
		CPack pack(req);
		pack.show();
		return pack;
	}

	static CPack mkWebSocketFrameReq()
	{
		char request[2008] = { '\0' };
		request[0] = char(0x81);
		request[1] = char(0xFE);		//126
		request[2] = char(0x00);		//描述长度
		request[3] = char(0x0F);		//固定发14字节数据+一个字符串终结符

										//以下4字节描述Masking-Key，防止本次数据传输完毕，连接被server关闭
		request[4] = char(0x41);
		request[5] = char(0x41);
		request[6] = char(0x41);
		request[7] = char(0x41);

		//填充14字节数据 + NUL
		char *pData = request + 8;
		strcat(pData, "hi, are you ok");
		std::cout << "@ws_request:" << std::endl;
		CPack pack(request, 23);
		pack.show();
		return pack;
	}
};
#endif