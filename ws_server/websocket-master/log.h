#ifndef __LOG__
#define __LOG__

#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdarg>

void log4debug(const char *msg, ...);

//日志单例
//fixme: 非线程安全的
class CLogger 
{
  public:
	static CLogger* getLogger();
	void write(const char *msg);
  private:
	CLogger();
	~CLogger();
	void createLogFile();
  private:
	static CLogger *mPtrLog;
	time_t mtim;
	struct tm *mt;
	struct tm mLastLogAt;
	FILE	*mfp;
	char mFileName[256];
	char mdata[256];
};
#endif
