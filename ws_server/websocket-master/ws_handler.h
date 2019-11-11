#ifndef __WS_REQ_HANDLER__
#define __WS_REQ_HANDLER__


#include <iostream>
#include <map>
#include <sstream>

#include <arpa/inet.h>

#include "log.h"
#include "ws_request.h"

#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum EWS_STATUS 
{
	WSS_CLOSED = 0,
	WSS_CONNECTED,
};



class CWSHandler
{
  public:
	  CWSHandler(int fd);
	 ~CWSHandler();
	 int process(std::string &msg);
	 inline char *getbuff()
	 {
		 return mMsgBuff;
	 }
	 typedef std::map<std::string, std::string> req_header_t;
  private:
	int tryAccept();
	void penddingRespHeader(char *request);
	int parseRequest();
	int send(char *buff);
private:
	char mMsgBuff[2048];		//请求二进制数据
	EWS_STATUS mStatus;			//连接状态
	req_header_t mHeaders;		//握手头部
	int mContext;				//关联的用户连接fd
	CWSRequest *mRequest;		//本次ws请求
};
#endif
