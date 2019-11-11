#include <unistd.h>
#include "ws_handler.h"

CWSHandler::CWSHandler(int fd):
		mMsgBuff(), mStatus(WSS_CLOSED), mHeaders(), mContext(fd), mRequest(new CWSRequest())
{}

CWSHandler::~CWSHandler(){}

//处理业务请求
int CWSHandler::process(std::string &msg)
{
	if(mStatus == WSS_CLOSED)
	{
		std::cout << "握手: " << mContext << std::endl;
		return tryAccept();
	}
	if (mRequest->unpack(mMsgBuff))
	{
		std::cout << "解包失败: sock = " << this->mContext << std::endl;
		return -1;
	}
	std::cout << "处理: 用户[" << mContext << "]" << std::endl;
	mRequest->show();
	//转发
	msg = mMsgBuff;
	mRequest->clear();
	memset(mMsgBuff, 0, sizeof(mMsgBuff));
	return 0;
}

//应用层实现握手
int CWSHandler::tryAccept()
{
	char resp[1024] = {};
	mStatus = WSS_CONNECTED;
	this->parseRequest();
	this->penddingRespHeader(resp);
	memset(mMsgBuff, 0, sizeof(mMsgBuff));
	return this->send(resp);
}

//填充握手应答头部
void CWSHandler::penddingRespHeader(char *request)
{  
	if (request)
	{
		strcat(request, "HTTP/1.1 101 Switching Protocols\r\n");
		strcat(request, "Connection: upgrade\r\n");
		strcat(request, "Sec-WebSocket-Accept: ");
		strcat(request, "Upgrade: websocket\r\n\r\n");
	}
}

//解析http头部，并返回json消息
//fixme: 重写该函数
int CWSHandler::parseRequest()
{
	std::istringstream s(mMsgBuff);
	std::string request;

	std::getline(s, request);
	if (request[request.size()-1] == '\r') 
	{
		request.erase(request.end()-1);
	}else 
	{
		return -1;
	}
	std::string header;
	std::string::size_type end;

	while (std::getline(s, header) && header != "\r") 
	{
		if (header[header.size()-1] != '\r') 
		{
			continue; //end
		} else 
		{
			header.erase(header.end()-1);	//remove last char
		}

		end = header.find(": ",0);
		if (end != std::string::npos) 
		{
			std::string key = header.substr(0,end);
			std::string value = header.substr(end+2);
			mHeaders[key] = value;
		}
	}
	return 0;
}

//回复http应答
int CWSHandler::send(char *buff)
{
	return write(mContext, buff, strlen(buff));
}
