//ws解包
#ifndef __WS_REQUEST__
#define __WS_REQUEST__

#include <cstdint>
#include <string>
#include <arpa/inet.h>
#include "log.h"


//fixme:改造此类
class CWSRequest 
{
public:
	CWSRequest();
	~CWSRequest();
	int unpack(const char *msg);
	void show()const;
	void clear();

	//按ws协议顺序解包
private:
	int stepFin(const char *msg, int &pos);
	int stepOpCode(const char *msg, int &pos);
	int stepMask(const char *msg, int &pos);
	int stepMaskingKey(const char *msg, int &pos);
	int stepPayloadSize(const char *msg, int &pos);
	int stepPayload(const char *msg, int &pos);
	std::string takeBuff()const;

private:
	uint8_t  mFin;
	uint8_t  mOpCode;
	uint8_t  mMask;
	uint8_t  mMaskingKey[4];
	uint64_t mszPayload;
	char	 mPayload[2048];
};
#endif
