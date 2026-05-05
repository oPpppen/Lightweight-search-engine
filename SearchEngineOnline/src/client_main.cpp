#include "Codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>

using namespace muduo;
using namespace muduo::net;

namespace {

class SearchClient {
public:
    SearchClient(EventLoop* loop, const InetAddress& serverAddr)
        : client_(loop, serverAddr, "SearchClient")
    {
        client_.setConnectionCallback(
            std::bind(&SearchClient::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
            std::bind(&SearchClient::onMessage, this,
                      std::placeholders::_1,
                      std::placeholders::_2,
                      std::placeholders::_3));
    }

    void connect()
    {
        client_.connect();
    }

    void disconnect()
    {
        client_.disconnect();
    }

    bool waitUntilConnected()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return cond_.wait_for(lock, std::chrono::seconds(5), [this] {
            return connected_;
        });
    }

    bool waitForResponse(std::string& response)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        const bool ready = cond_.wait_for(lock, std::chrono::seconds(5), [this] {
            return !responses_.empty() || !connected_;
        });

        if (!ready || responses_.empty()) {
            return false;
        }

        response = responses_.front();
        responses_.pop_front();
        return true;
    }

    void sendRequest(uint8_t type, const std::string& query)
    {
        TcpConnectionPtr conn;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            conn = connection_;
        }

        if (!conn) {
            std::cout << "server is not connected" << std::endl;
            return;
        }

        std::string packet;
        Codec::encode(type, query, packet);
        conn->send(packet);
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (conn->connected()) {
                connection_ = conn;
                connected_ = true;
            } else {
                connection_.reset();
                connected_ = false;
                responses_.clear();
            }
        }

        cond_.notify_all();

        if (conn->connected()) {
            std::cout << "connected to " << conn->peerAddress().toIpPort() << std::endl;
        } else {
            std::cout << "disconnected from server" << std::endl;
        }
    }

    void onMessage(const TcpConnectionPtr&, Buffer* buffer, Timestamp)
    {
        Message response;
        while (Codec::tryDecode(buffer, response)) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                responses_.push_back(response.value);
            }
            cond_.notify_all();
        }
    }

private:
    TcpClient client_;
    TcpConnectionPtr connection_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool connected_ = false;
    std::deque<std::string> responses_;
};

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return 1;
    }

    muduo::Logger::setLogLevel(muduo::Logger::WARN);

    const std::string ip = argv[1];
    const uint16_t port = static_cast<uint16_t>(std::strtoul(argv[2], nullptr, 10));

    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();

    InetAddress serverAddr(ip, port);
    SearchClient client(loop, serverAddr);
    client.connect();

    if (!client.waitUntilConnected()) {
        std::cerr << "connect timeout" << std::endl;
        loop->quit();
        return 1;
    }

    while (true) {
        std::cout << "请输入请求类型：" << std::endl;
        std::cout << "1. 关键字推荐" << std::endl;
        std::cout << "2. 网页搜索" << std::endl;
        std::cout << "0. 退出" << std::endl;
        std::cout << "type: ";
        std::string typeLine;
        if (!std::getline(std::cin, typeLine)) {
            break;
        }

        if (typeLine.empty()) {
            continue;
        }

        char* end = nullptr;
        const long type = std::strtol(typeLine.c_str(), &end, 10);
        if (end == typeLine.c_str() || *end != '\0') {
            std::cout << "invalid type" << std::endl;
            continue;
        }

        if (type == 0) {
            break;
        }

        if (type < 0 || type > 255) {
            std::cout << "type should be between 0 and 255" << std::endl;
            continue;
        }

        std::cout << "query: ";
        std::string query;
        if (!std::getline(std::cin, query)) {
            break;
        }

        client.sendRequest(static_cast<uint8_t>(type), query);

        std::string response;
        if (client.waitForResponse(response)) {
            std::cout << response << std::endl;
        } else {
            std::cout << "wait response timeout" << std::endl;
        }
    }

    client.disconnect();
    loop->quit();
    return 0;
}
