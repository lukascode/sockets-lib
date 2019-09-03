#pragma once

#include <thread>
#include <algorithm>
#include "Address.h"
#include "TcpConnectionHandler.h"
#include "ThreadPool.h"
#include <functional>
#include "NanoException.h"

class TcpServer
{
public:
	
	/// Creates tcp server 
	TcpServer(std::function<TcpConnectionHandler*()> connHandlerFactory);

	~TcpServer();

	/// Bind to all interfaces on the provided port and listen on incoming connections
	void Listen(short port);

	/// Bind to interface provided by ip on the provided port
	void Listen(std::string ip, short port);

	/// Check whether the server is already in listen mode
	bool isListening();

	/// Disconnect client socket
	bool Disconnect(Socket *client);

	/// Sends data to all clients
	void Broadcast(std::string &data) const; 

	/// Sets number of threads in the pool which are used for handling incoming connections
	void setThreadPoolSize(int size);

	/// Stops tcp server
	void Stop();

private:
	static const int defaultThreadPoolSize = 20;
	ThreadPool* tp;
	int tpSize;
	std::vector<Socket *> clients;
	std::function<TcpConnectionHandler*()> connHandlerFactory;
	Socket *socket;
	short port;
	std::string ip;
	std::atomic<bool> halted;
	std::atomic<bool> listening;

	void _Listen();

	void Clean();
};

class TcpServerException: public NanoException
{
public:
	TcpServerException(std::string msg): NanoException(msg) {}
};