#ifndef __WS_FRAMEWORK__
#define __WS_FRAMEWORK__

#include <queue>
#include "ws_handler.h"

#define TIMEWAIT 100
#define N_MAX_PACK_SIZE 2048
#define N_MAX_EVENTS_SIZE 20



//������
//fixme: �̲߳���ȫ��
class CFrameworkService 
{
public:
	//fixme
	typedef std::map<int, CWSHandler*> user_context_map_t;
	void run();
	static CFrameworkService* getServerInstance(const char *ip, const char *port);
private:
	//����Ĭ�Ϲ���
	CFrameworkService(const char *ip, const char *port);
	~CFrameworkService();
	int init(const char *ip, const char *port);
	int epolling();
	void transfer(const std::string &msg);
	int setNonblockIO(int fd);
	void ctlEventOnSocket(int fd, bool flag);
	void reCtlEventOnSocket(const int &fd);
	void restartAll();
	bool write(int sockfd, const char* buffer, int len);
private:
	int mEpollFd;
	int mListenFd;
	
	user_context_map_t mUserHandlers;
	std::queue<std::string> mMsgQueue;
	//todo ����boost����ָ��
	static CFrameworkService *mPtrInstance;
};
#endif
