#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <memory>
#include <map>

//system
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include "log.h"
#include "framework.h"

CFrameworkService* CFrameworkService::mPtrInstance = NULL;

CFrameworkService::CFrameworkService(const char *ip, const char *port):
	mEpollFd(0), mListenFd(0), mUserHandlers(), mMsgQueue()
{
	if (this->init(ip, port))
	{
		exit(-1);
	}
	log4debug("ws_im 服务运行中....");
}

CFrameworkService::~CFrameworkService()
{
	std::cout << "ws server stoped..." << std::endl;
}

int CFrameworkService::init(const char *ip_, const char *port_)
{
	assert(ip_ && port_);
	this->mListenFd = socket(AF_INET, SOCK_STREAM, 0);
	if( -1 == mListenFd)
	{
		log4debug("create listen socket failed");
		return -1;
	}

	uint16_t _port = uint16_t(strtoul(port_, 0, 10));
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(sockaddr_in));
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, ip_, &server_addr.sin_addr);
	server_addr.sin_port = htons(_port);

	if(-1 == bind(mListenFd, (struct sockaddr *)(&server_addr), sizeof(server_addr)))
	{
		log4debug("bind network addr for listenfd failed");
		return -1;
	}
	if(-1 == listen(mListenFd, 5)){
		log4debug("set size of listen-queue failed");
		return -1;
	}

	mEpollFd = epoll_create(N_MAX_EVENTS_SIZE);
	ctlEventOnSocket(mListenFd, true);
	return 0;
}

int CFrameworkService::epolling()
{
	int nfds = 0;
	int fd = 0;
	int bufflen = 0;
	struct sockaddr_in clientAddr;
	socklen_t szClntAddr;
	struct epoll_event events[N_MAX_EVENTS_SIZE];

	while(true)
	{
		nfds = epoll_wait(mEpollFd, events, N_MAX_EVENTS_SIZE, TIMEWAIT);
		for(int i = 0; i < nfds; i++)
		{
			if(events[i].data.fd == mListenFd)
			{
				fd = accept(mListenFd, (struct sockaddr *)&clientAddr, &szClntAddr);
				this->ctlEventOnSocket(fd, true);
			}
			else if(events[i].events & EPOLLIN)
			{
				if ((fd = events[i].data.fd) < 0)
				{
					continue;
				}
				//fixme
				CWSHandler *handler = mUserHandlers[fd];
				if (handler == NULL)
				{
					ctlEventOnSocket(fd, false);
					continue;
				}
				if((bufflen = read(fd, handler->getbuff(), N_MAX_PACK_SIZE)) <= 0)
				{
					this->ctlEventOnSocket(fd, false);
					continue;
				}
				else
				{
					//todo 改为封装业务请求插入任务队列，以线程池实现
					std::string theMsg;
					handler->process(theMsg);
					if (!theMsg.empty())
					{
						mMsgQueue.push(theMsg);
					}
					this->reCtlEventOnSocket(events[i].data.fd);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				if (!mMsgQueue.empty())
				{
					std::string msg = mMsgQueue.front();
					mMsgQueue.pop();
					this->transfer(msg);
				}
				restartAll();
			}
		}
	}
	return 0;
}



//转发
void CFrameworkService::transfer(const std::string &msg)
{
	for (user_context_map_t::const_iterator it = mUserHandlers.begin(); it != mUserHandlers.end(); it++)
	{
		if (it->second)
		{
			this->write(it->first, msg.c_str(), msg.size());
			this->reCtlEventOnSocket(it->first);
		}
	}
}

void CFrameworkService::restartAll()
{
	for (user_context_map_t::const_iterator it = mUserHandlers.begin(); it != mUserHandlers.end(); it++)
	{
		if (it->second)
		{
			this->reCtlEventOnSocket(it->first);
		}
	}
}

void CFrameworkService::reCtlEventOnSocket(const int &sockfd)
{
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR;
	event.data.fd = sockfd;
	epoll_ctl(mEpollFd, EPOLL_CTL_MOD, sockfd, &event);
}

bool CFrameworkService::write(int sockfd, const char* buffer, int len)
{
	int _len = len;
	int bytes_write = 0;
	while (1)
	{
		bytes_write = send(sockfd, buffer, len, 0);
		if (bytes_write == -1)
		{
			return false;
		}
		else if (bytes_write == 0)
		{
			return false;
		}

		len -= bytes_write;
		buffer = buffer + bytes_write;
		if (len <= 0)
		{
			std::cout << "@@转发[" << _len << "] bytes to 用户[" << sockfd << "]" << std::endl;
			return true;
		}
	}
}


int CFrameworkService::setNonblockIO(int fd)
{
	int old;
	if ((old = fcntl(fd, F_GETFL, 0)) == -1)
	{
		old = 0;
	}
    return fcntl(fd, F_SETFL, old | O_NONBLOCK);
}

CFrameworkService*  CFrameworkService::getServerInstance(const char *ip, const char *port)
{
	if (NULL == mPtrInstance)
	{
		mPtrInstance = new CFrameworkService(ip, port);
	}
	return mPtrInstance;
}

void CFrameworkService::ctlEventOnSocket(int fd, bool flag)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = flag ? (EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR) : 0;
	epoll_ctl(mEpollFd, flag ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, fd, &ev);
	if(flag)
	{
		this->setNonblockIO(fd);
		mUserHandlers[fd] = new CWSHandler(fd);
		if (fd != mListenFd)
		{
			std::cout << "新用户连上了: sock = " << fd << std::endl;
		}
	}
	else
	{
		delete mUserHandlers[fd];
		mUserHandlers.erase(fd);
		close(fd);
		std::cout << "用户退出群聊: sock = " << fd << std::endl;
	}
}

void CFrameworkService::run()
{
	this->epolling();
}
