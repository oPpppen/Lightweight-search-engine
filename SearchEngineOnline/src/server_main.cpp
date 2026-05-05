#include "../include/SearchServer.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
    uint16_t port = 8888;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::strtoul(argv[1], nullptr, 10));
    }

    muduo::Logger::setLogLevel(muduo::Logger::WARN);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(port);

    const std::string dictFile = "data/dict.dat";
    const std::string indexFile = "data/index.dat";
    const std::string pageLibFile = "data/pagelib.dat";
    const std::string offsetLibFile = "data/offset.dat";
    const std::string invertedIndexFile = "data/inverted_index.dat";
    const std::string stopWordsFile = "data/stop_words.txt";

    SearchServer server(&loop,
                        listenAddr,
                        dictFile,
                        indexFile,
                        pageLibFile,
                        offsetLibFile,
                        invertedIndexFile,
                        stopWordsFile);
    server.start();

    std::cout << "search_server listening on 0.0.0.0:" << port << std::endl;
    loop.loop();
    return 0;
}
