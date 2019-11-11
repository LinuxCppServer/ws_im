#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <ostream>
#include <iostream>


#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#include "pack.h"


int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epoll_fd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLOUT | EPOLLET | EPOLLERR;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

bool write_nbytes(int sockfd, const char* buffer, int len)
{
	const char *base = buffer;
	int _len = len;
	int bytes_write = 0;
	
	while (1)
	{
		bytes_write = send(sockfd, buffer, len, 0);
		if (bytes_write == -1)
		{
			delete [] base;
			return false;
		}
		else if (bytes_write == 0)
		{
			delete [] base;
			return false;
		}

		len -= bytes_write;
		buffer = buffer + bytes_write;
		if (len <= 0)
		{
			std::cout << "send[" << _len << "] bytes to [" << sockfd << "]" << std::endl;
			delete [] base;
			return true;
		}
	}
}

bool read_once(int sockfd, char* buffer, int len)
{
	int bytes_read = 0;
	memset(buffer, '\0', len);
	bytes_read = recv(sockfd, buffer, len, 0);
	if (bytes_read == -1)
	{
		return false;
	}
	else if (bytes_read == 0)
	{
		return false;
	}
	printf("read in %d bytes from socket %d with content: %s\n", bytes_read, sockfd, buffer);

	return true;
}

void start_conn(int epoll_fd, int num, const char* ip, int port)
{
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	for (int i = 0; i < num; ++i)
	{
		sleep(1);
		int sockfd = socket(PF_INET, SOCK_STREAM, 0);

		if (sockfd < 0)
		{
			continue;
		}
		std::cout << "create user[" << sockfd << "]" << std::endl;
		if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) == 0)
		{
			CPack handshak = CPack::mkWebSocketHandShakeReq(ip, port);
			if (!write_nbytes(sockfd, handshak.info(), handshak.size()))
			{
				std::cout << "handsake with [" << ip << ", " << port << "] failed" << std::endl;
				continue;
			}
			std::cout << "handshaked with server: client[" << sockfd << "] success!" << std::endl;
			addfd(epoll_fd, sockfd);
		}
	}
}

void close_conn(int epoll_fd, int sockfd)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, 0);
	close(sockfd);
	std::cout << "user[" << sockfd << "] closed" << std::endl;
}

int main(int argc, char* argv[])
{
	assert(argc == 4);
	int epoll_fd = epoll_create(100);
	start_conn(epoll_fd, atoi(argv[3]), argv[1], atoi(argv[2]));
	epoll_event events[10000];
	char buffer[2048];
	while (1)
	{
		int fds = epoll_wait(epoll_fd, events, 10000, 2000);
		for (int i = 0; i < fds; i++)
		{
			int sockfd = events[i].data.fd;
			if (events[i].events & EPOLLIN)
			{
				if (!read_once(sockfd, buffer, 2048))
				{
					close_conn(epoll_fd, sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLOUT)
			{
				CPack req = CPack::mkWebSocketFrameReq();
				if (!write_nbytes(sockfd, req.info(), req.size()))
				{
					std::cout << "user[" << sockfd << "] send with: " << req << " failed" << std::endl;
					close_conn(epoll_fd, sockfd);
				}
				std::cout << "user[" << sockfd << "] send with: " << req << " data size[" << req.size() << "]" << std::endl;
				struct epoll_event event;
				event.events = EPOLLIN | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLERR)
			{
				close_conn(epoll_fd, sockfd);
			}
		}
	}
}