#include "../include/SearchServer.h"

#include "../include/Codec.h"

#include <muduo/net/Buffer.h>

#include <functional>
#include <iostream>
#include <sstream>

using namespace muduo;
using namespace muduo::net;

namespace {

uint8_t responseTypeFor(const Message& msg)
{
    if (msg.type == 1 || msg.type == 2) {
        return msg.type;
    }
    return 255;
}

} // namespace

SearchServer::SearchServer(EventLoop* loop,
                           const InetAddress& listenAddr,
                           const std::string& dictFile,
                           const std::string& indexFile,
                           const std::string& pageLibFile,
                           const std::string& offsetLibFile,
                           const std::string& invertedIndexFile,
                           const std::string& stopWordsFile)
    : server_(loop, listenAddr, "SearchServer")
    , keywordRecommender_(dictFile, indexFile)
    , webSearcher_(pageLibFile, offsetLibFile, invertedIndexFile, stopWordsFile)
{
    server_.setConnectionCallback(
        std::bind(&SearchServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&SearchServer::onMessage, this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));
    server_.setThreadNum(2);
}

void SearchServer::start()
{
    server_.start();
}

void SearchServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        std::cout << "client connected: " << conn->peerAddress().toIpPort() << std::endl;
    } else {
        std::cout << "client disconnected: " << conn->peerAddress().toIpPort() << std::endl;
    }
}

void SearchServer::onMessage(const TcpConnectionPtr& conn,
                             Buffer* buffer,
                             Timestamp)
{
    Message request;
    while (Codec::tryDecode(buffer, request)) {
        const std::string responseJson = handleRequest(request);
        std::string responsePacket;
        Codec::encode(responseTypeFor(request), responseJson, responsePacket);
        conn->send(responsePacket);
    }
}

std::string SearchServer::handleRequest(const Message& msg)
{
    if (msg.type == 1) {
        return keywordRecommender_.recommend(msg.value, 5);
    }
    if (msg.type == 2) {
        return webSearcher_.search(msg.value, 10);
    }
    return "{\"type\":\"error\",\"message\":\"unknown request type\"}";
}
