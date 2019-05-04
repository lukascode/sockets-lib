#ifndef SOCKET_H
#define SOCKET_H

#include "NetworkUtils.h"
#include "Address.h"
#include <exception>
#include <vector>
#include <mutex>
#include <cstddef>

class Socket 
{
public:
	static Socket* createSocket(int type); /* type SOCK_STREAM / SOCK_DGRAM */
	Socket(int socket_descriptor);
	Socket(const Socket& socket) = delete;
	~Socket();

	Address getRemoteAddress();
	Address* getBoundAddress();

	int getSocket();
	void setSocket(int socket_descriptor);
	void closeSocket();
	int getSocketType();

	void _bind(Address* address);
	void _listen(int backlog);
	Socket* _accept();

	void sendall(const std::vector<uint8_t>& data);
	std::vector<uint8_t> recvall(size_t size);

	std::vector<uint8_t> recvuntil(const std::string pattern, size_t maxlen);
	std::vector<uint8_t> recvuntil(const std::vector<uint8_t>& pattern, size_t maxlen);

	void sendall(const uint8_t* buf, size_t len);
	void recvall(uint8_t* buf, size_t len);

	void recvuntil(uint8_t* buf, size_t buflen, const uint8_t* pattern, size_t patternlen, size_t* len);
	int recvtimeout(int s, uint8_t* buf, int len, int timeout);

private:
	int socket_descriptor;
	Address* boundAddress;

	std::mutex _send;
	std::mutex _recv;
	std::mutex _recvuntil;

	int isContainPattern(const uint8_t* buf, size_t len, const uint8_t* pattern, size_t patternlen);
	bool isValidDescriptor();
};

class SocketException : public std::runtime_error 
{
public:
	SocketException(std::string msg): std::runtime_error(msg) {}
};

class SendException: public SocketException 
{
public:
	SendException(std::string msg): SocketException(msg) {}
};

class RecvException: public SocketException
{
public:
	RecvException(std::string msg): SocketException(msg) {}
};

class SocketConnectionClosedException: public SocketException
{
public:
	SocketConnectionClosedException(std::string msg): SocketException(msg) {}
};

#endif /* SOCKET_H */
