#include <cstdlib>
#include <iostream>
#include "log.h"


void log4debug(const char *msg, ...) 
{
	char mdata[256] = {'\0'};
	va_list args;
	va_start(args, msg);
	vsprintf(mdata, msg, args);
	va_end(args);
	CLogger::getLogger()->write(mdata);
}

CLogger *CLogger::mPtrLog = NULL;

CLogger::CLogger():
	mtim(0), mt(NULL), mLastLogAt(), mfp(NULL), mFileName(), mdata()
{
	//fixme: 设置日志文件指针
	#ifdef __WRITE_FILE__
		create_log_file();
	#endif
}

CLogger::~CLogger()
{
	#ifdef __WRITE_FILE__
		fclose(fp);
	#endif
}

void CLogger::createLogFile()
{
	if (mfp != NULL)
	{
		fclose(mfp);
	}

	sprintf(mFileName, "./log/log_");
	time(&mtim); 
	mt = localtime(&mtim);
	memcpy(&mLastLogAt, mt, sizeof(struct tm));
	sprintf(mFileName + 15, "%02d_%02d", mt->tm_mon + 1, mt->tm_mday); 
	mfp = fopen(mFileName, "a+");
	if (NULL == mfp)
	{
		std::cout << "log set up failed" << std::endl;
		exit(-1);
	}
	std::cout << "log set up" << std::endl;
}

//fixme: 线程不安全的
CLogger *CLogger::getLogger()
{
	if(NULL == mPtrLog)
	{
		mPtrLog = new CLogger();
	}
	return mPtrLog;
}

void CLogger::write(const char *msg)
{
	time(&mtim); 
	mt = localtime(&mtim); 
	sprintf(mdata, "[%02d:%02d:%02d] %s\n", mt->tm_hour, mt->tm_min, mt->tm_sec, msg);
	#ifdef __WRITE_FILE__
	  if (mt->tm_mday != mLastLogAt.tm_mday ||
		mt->tm_mon != mLastLogAt.tm_mon ||
		mt->tm_year != mLastLogAt.tm_year)
	  {
	 	createLogFile()
	  }
	#endif
	std::cout << std::endl << mdata;
	std::cout.flush();
}

