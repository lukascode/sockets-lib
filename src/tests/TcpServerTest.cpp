#include "catch.hpp"
#include "../socknano.h"
#include <functional>
#include <atomic>
#include "TestUtils.h"

TEST_CASE("tcp server general test", "[tcp-server]")
{

    class Handler : public TcpConnectionHandler
    {
    public:
        std::atomic<bool> &handlerStarted;
        Handler(std::atomic<bool> &handlerStarted) : handlerStarted(handlerStarted) {}
        virtual void HandleConnection() { handlerStarted = true; }
    };

    std::atomic<bool> handlerStarted(false);

    uint16_t port = RandomPort();
    auto server = TcpServer::Create([&handlerStarted] { return std::make_shared<Handler>(handlerStarted); });

    std::thread serverThread([server, port] {
        server->Listen(port);
    });
    serverThread.detach();

    // wait for server
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    try
    {
        server->Listen(port);
        FAIL_CHECK("Expected TcpServerException");
    }
    catch (TcpServerException &e)
    {
        REQUIRE(std::string(e.what()) == "Already listening");
    }

    auto socket = Socket::Create(SOCK_STREAM);
    socket->Connect(std::make_shared<Address>(port));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE(server->IsListening());
    REQUIRE(handlerStarted.load());

    server->Stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE(!server->IsListening());
}

TEST_CASE("should transfer data", "[tcp-server]")
{
    std::string PING = "PING";
    std::string PONG = "PONG";

    class Handler : public TcpConnectionHandler
    {
    public:
        std::string &dataReceived;
        Handler(std::string &dataReceived) : dataReceived(dataReceived) {}
        virtual void HandleConnection()
        {
            auto data = socket->RecvAll(4);
            socket->SendAll("PONG");
            dataReceived = std::string(data.begin(), data.end());
        }
    };

    uint16_t port = RandomPort();
    std::string dataReceivedByServer;
    auto server = TcpServer::Create([&dataReceivedByServer] { return std::make_shared<Handler>(dataReceivedByServer); });

    std::thread serverThread([server, port] {
        server->Listen(port);
    });
    serverThread.detach();

    // wait for server
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE(server->IsListening());

    auto socket = Socket::Create(SOCK_STREAM);
    socket->Connect(std::make_shared<Address>(port));
    socket->SendAll(PING);

    auto data = socket->RecvAll(4);
    std::string dataReceivedByClient = std::string(data.begin(), data.end());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE(dataReceivedByServer == PING);
    REQUIRE(dataReceivedByClient == PONG);

    server->Stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE(!server->IsListening());
}

TEST_CASE("should broadcast", "[tcp-server]")
{

    std::string messageToBroadcast = "tq0weijgansdg0e9rtigjvmsrfg";

    class Handler : public TcpConnectionHandler
    {
    public:
        virtual void HandleConnection()
        {
            // keep connected
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
    };

    uint16_t port = RandomPort();
    auto server = TcpServer::Create([] { return std::make_shared<Handler>(); });

    std::thread serverThread([server, port] {
        server->Listen(port);
    });
    serverThread.detach();

    // wait for server
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE(server->IsListening());

    auto client1 = Socket::Create(SOCK_STREAM);
    auto client2 = Socket::Create(SOCK_STREAM);

    client1->Connect(std::make_shared<Address>(port));
    client2->Connect(std::make_shared<Address>(port));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE(server->GetNumberOfConnections() == 2);

    server->Broadcast(messageToBroadcast);

    auto dataReceivedByClient1 = client1->RecvAll(messageToBroadcast.size());
    auto dataReceivedByClient2 = client2->RecvAll(messageToBroadcast.size());

    REQUIRE(std::string(dataReceivedByClient1.begin(), dataReceivedByClient1.end()) == messageToBroadcast);
    REQUIRE(std::string(dataReceivedByClient2.begin(), dataReceivedByClient2.end()) == messageToBroadcast);

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    REQUIRE(server->GetNumberOfConnections() == 0);

    server->Stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE(!server->IsListening());
}