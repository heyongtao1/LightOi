#ifndef _TCP_CLIENT_BASE_H
#define _TCP_CLIENT_BASE_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <vector>
#include <string>
#include <atomic>
#include <iostream>
#include <chrono>
#include <errno.h>
#include "common_function.hpp"
#include "unable_copy.hpp"

class tcp_client_base
{
public:
    tcp_client_base() : m_sockfd (-1) {}

    void init(const std::string& serv_ip, int serv_port = 10588)
    {
        std::string ip = serv_ip;
        int port = serv_port;
        std::vector<std::string> v_res = com_fun::split(serv_ip, ":");
        if (v_res.size() == 2) {
            ip = v_res[0];
            port = std::atoi(v_res[1].c_str());
        }

        create_socketaddr(ip, port);                           // 创建tcp服务端地址信息
        m_connect_status.store(-1, std::memory_order_relaxed); // 初始化赋-1，表示未连接
    }

    ~tcp_client_base()
    {
        tcp_disconnect();                                  // 关闭socket
    }

    bool write_data(const std::string& data)
    {
        // 写入数据
        char buf[1024] = {};
        int data_size = data.size();
        int offset = 0;

        memcpy(buf + offset, &data_size, sizeof(data_size));
        offset = offset + sizeof(data_size);
        memcpy(buf + offset, data.data(), data_size);
        int total_size = data_size + sizeof(data_size);

        bool ret = send_data(buf, total_size);
        if (!ret) {
            return false;
        }

        return true;
    }

    bool read_data(std::string& data)
    {
        // 读取数据
        int data_len;
        bool ret = recv_data(&data_len, 4);
        if (!ret) {
            return false;
        }

        data.resize(data_len);
        ret = recv_data(data.data(), data_len);
        if (!ret) {
            return false;
        }

        return true;
    }

    void tcp_connect()
    {
        m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        m_connect_status.store(connect(m_sockfd, reinterpret_cast<struct sockaddr*>(&m_servaddr), sizeof(m_servaddr)), std::memory_order_relaxed);
    }

    void tcp_disconnect()
    {
        close(m_sockfd); // 关闭客户端
        m_sockfd = -1;
        m_connect_status.store(-1, std::memory_order_relaxed);
    }

private:
    bool send_data(const void* data, int len)
    {
        if (m_connect_status.load(std::memory_order_relaxed)) { // 当前状态若为断开，则通知线程连接服务器，本次写失败
            return false;
        }

        int ret = 0;
        int offset = 0;
        while (len) { // 循环写数据，直至全部写入
            ret = send(m_sockfd, data + offset, len, 0);
            if (ret == 0) {
                if (errno != EAGAIN) {
                    return false;
                }
            }
            else if (ret == -1) {
                if (errno != EAGAIN) {
                    return false;
                }
                else {
                    ret = 0;
                }
            }
            len -= ret;
            offset += ret;
        }

        return true;
    }

    bool recv_data(void* data, int len)
    {
        if (m_connect_status.load(std::memory_order_relaxed)) { // 当前状态若为断开，则通知线程连接服务器，本次写失败
            return false;
        }

        int ret = 0;
        int offset = 0;
        while (len) { // 循环读数据，直至全部读完
            ret = recv(m_sockfd, data + offset, len, 0);
            if (ret == 0) {
                if (errno != EAGAIN) {
                    return false;
                }
            }
            else if (ret == -1) {
                if (errno != EAGAIN) {
                    return false;
                }
                else {
                    ret = 0;
                }
            }
            len -= ret;
            offset += ret;
        }

        return true;
    }

    void create_socketaddr(const std::string& serv_ip, int serv_port)
    {
        bzero(&m_servaddr, sizeof(m_servaddr));
        m_servaddr.sin_family = AF_INET;
        m_servaddr.sin_port = htons(serv_port);
        inet_aton(serv_ip.c_str(), &m_servaddr.sin_addr);
    }

protected:
    int m_sockfd;                                    // 客户端句柄
    struct sockaddr_in m_servaddr;                   // 服务端地址
    std::atomic<int> m_connect_status;               // 连接状态
};

#endif