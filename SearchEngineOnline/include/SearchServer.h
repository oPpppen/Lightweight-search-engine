#pragma once

#include "KeywordRecommender.h"
#include "Message.h"
#include "WebSearcher.h"

#include <muduo/base/Timestamp.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>

#include <string>

class SearchServer {
public:
    SearchServer(muduo::net::EventLoop* loop,
                 const muduo::net::InetAddress& listenAddr,
                 const std::string& dictFile,
                 const std::string& indexFile,
                 const std::string& pageLibFile,
                 const std::string& offsetLibFile,
                 const std::string& invertedIndexFile,
                 const std::string& stopWordsFile);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buffer,
                   muduo::Timestamp receiveTime);
    std::string handleRequest(const Message& msg);

private:
    muduo::net::TcpServer server_;
    KeywordRecommender keywordRecommender_;
    WebSearcher webSearcher_;
};
