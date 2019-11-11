#include <iostream>
#include "ws_request.h"

CWSRequest::CWSRequest():
		mFin(), mOpCode(), mMask(), mMaskingKey(), mszPayload(), mPayload(){}

CWSRequest::~CWSRequest(){}

//unpack
int CWSRequest::unpack(const char *msg)
{
	int ret;
	int pos = 0;
	if (ret = this->stepFin(msg, pos))			 return ret;
	if ((ret = this->stepOpCode(msg, pos)))		 return ret;
	if ((ret = this->stepMask(msg, pos)))		 return ret;
	if ((ret = this->stepPayloadSize(msg, pos))) return ret;
	if ((ret = this->stepMaskingKey(msg, pos)))	 return ret;
	return this->stepPayload(msg, pos);
}

void CWSRequest::show()const
{
	log4debug("WEBSOCKET PROTOCOL\n"
				"FIN: %d\n"
				"OPCODE: %d\n"
				"MASK: %d\n"
		        "MASKING-KEY: %s\n"
				"PAYLOADLEN: %d\n"
				"PAYLOAD: %s",
				mFin, mOpCode, mMask, mMaskingKey,mszPayload, mPayload);
}

void CWSRequest::clear(){
	mFin = 0;
	mOpCode = 0;
	mMask = 0;
	memset(mMaskingKey, 0, sizeof(mMaskingKey));
	mszPayload = 0;
	memset(mPayload, 0, sizeof(mPayload));
}

//Fin
int CWSRequest::stepFin(const char *msg, int &pos)
{
	int reason = -1;
	if (msg)
	{
		mFin = (unsigned char)msg[pos] >> 7;
		reason++;
	}
	return reason;
}

//opcode
int CWSRequest::stepOpCode(const char *msg, int &pos)
{
	int reason = -1;
	if (msg)
	{
		mOpCode = msg[pos] & 0x0f;
		//不自定义数据格式，rsv1-rsv3之间位不用，直接跳过
		pos++;
		reason++;
	}
	return reason;
}

//Mask
int CWSRequest::stepMask(const char *msg, int &pos) 
{
	int reason = -1;
	if (msg)
	{
		mMask = (unsigned char)msg[pos] >> 7;
		reason++;
	}
	return reason;
}

//Payload length
int CWSRequest::stepPayloadSize(const char *msg, int &pos) 
{

	int reason = -1;
	if (msg)
	{
		mszPayload = msg[pos] & 0x7f;
		pos++;
		if (mszPayload == 126) 
		{
			uint16_t length = 0;
			memcpy(&length, msg + pos, 2);
			pos += 2;
			mszPayload = ntohs(length);
			std::cout << "@126: unpacked len: =>" << mszPayload << std::endl;

		}
		else if (mszPayload == 127) 
		{
			//fixme： 貌似这里应该+8
			uint32_t length = 0;
			memcpy(&length, msg + pos, 8);
			pos += 8;
			mszPayload = ntohl(length);
			std::cout << "@127: unpacked len: =>" << mszPayload << std::endl;
		}
		reason = 0;
	}
	return reason;
}


//Masking-key
int CWSRequest::stepMaskingKey(const char *msg, int &pos)
{
	int reason = -1;
	if (msg)
	{
		if (mMask != 1)	 return 0;
		for (int i = 0; i < 4; i++)
		{
			mMaskingKey[i] = msg[pos + i];
		}
		pos += 4;
		reason++;
	}
	return reason;
}


//payload
int CWSRequest::stepPayload(const char *msg, int &pos)
{
	int reason = -1;
	if (msg)
	{
		memset(mPayload, 0, sizeof(mPayload));
		if (mMask != 1) 
		{
			memcpy(mPayload, msg + pos, mszPayload);
		}
		else 
		{
			for (uint64_t i = 0; i < mszPayload; i++) 
			{
				int j = i % 4;
				mPayload[i] = msg[pos + i] ^ mMaskingKey[j];
			}
		}
		pos += mszPayload;
		reason++;
	}
	return reason;
}


//payload
std::string CWSRequest::takeBuff()const
{
	std::string data;
	if (mPayload)
	{
		char buff[2048] = { '\0' };
		memset(buff, 0, sizeof(buff));
		if (mMask != 1)
		{
			memcpy(buff, mPayload, mszPayload);
		}
		else
		{
			for (uint64_t i = 0; i < mszPayload; i++)
			{
				int j = i % 4;
				buff[i] = mPayload[i] ^ mMaskingKey[j];
			}
		}
		data = buff;
	}
	return data;
}