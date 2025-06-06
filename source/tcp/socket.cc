#include "socket.h"

Socket::Socket() : _sockfd(-1) {}

Socket::Socket(int fd) : _sockfd(fd) {}

Socket::~Socket()
{
    Close();
}

int Socket::Fd()
{
    return _sockfd;
}

bool Socket::Create()
{
    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sockfd < 0)
    {
        LOG_ERROR("CREATE SOCKET FAILED!!");
        return false;
    }
    return true;
}

bool Socket::Bind(const std::string &ip, uint16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t len = sizeof(struct sockaddr_in);
    int ret = bind(_sockfd, (struct sockaddr *)&addr, len);
    if (ret < 0)
    {
        LOG_ERROR("BIND ADDRESS FAILED!");
        return false;
    }
    return true;
}

bool Socket::Listen(int backlog)
{
    int ret = listen(_sockfd, backlog);
    if (ret < 0)
    {
        LOG_ERROR("SOCKET LISTEN FAILED!");
        return false;
    }
    return true;
}

bool Socket::Connect(const std::string &ip, uint16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t len = sizeof(struct sockaddr_in);
    int ret = connect(_sockfd, (struct sockaddr *)&addr, len);
    if (ret < 0)
    {
        LOG_ERROR("CONNECT SERVER FAILED!");
        return false;
    }
    return true;
}

int Socket::Accept()
{
    int newfd = accept(_sockfd, NULL, NULL);
    if (newfd < 0)
    {
        LOG_ERROR("SOCKET ACCEPT FAILED!");
        return -1;
    }
    return newfd;
}

ssize_t Socket::Recv(void *buf, size_t len, int flag)
{
    errno = 0;
    ssize_t ret = recv(_sockfd, buf, len, flag);
    if (ret <= 0)
    {
        if (ret == 0)
        {
            LOG_INFO("client quit ...........");
            return 0;
        }
        else if (ret == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return -1;
            }
            LOG_ERROR("SOCKET RECV FAILED!! ret : %ld errno: %d errorms: %s ", ret, errno, strerror(errno));

            return -1;
        }
    }
    return ret;
}

ssize_t Socket::NonBlockRecv(void *buf, size_t len)
{
    return Recv(buf, len, MSG_DONTWAIT);
}

ssize_t Socket::Send(const void *buf, size_t len, int flag)
{
    ssize_t ret = send(_sockfd, buf, len, flag);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            return ret;
        }
        LOG_ERROR("SOCKET SEND FAILED!!");
        return ret;
    }
    return ret;
}

ssize_t Socket::NonBlockSend(void *buf, size_t len)
{
    if (len == 0)
        return 0;
    return Send(buf, len, MSG_DONTWAIT);
}

void Socket::Close()
{
    if (_sockfd != -1)
    {
        close(_sockfd);
        _sockfd = -1;
    }
}

bool Socket::CreatListenSocket(const std::string &ip, uint16_t port, bool block_flag, bool reusePort)
{
    if (Create() == false)
        return false;
    NonBlock();
    if (reusePort)
        ReuseAddress();
    if (Bind(ip, port) == false)
        return false;
    if (Listen() == false)
        return false;
    // ReuseAddress();
    return true;
}

bool Socket::CreateClient(const std::string &ip, uint16_t port)
{
    if (Create() == false)
        return false;
    if (Connect(ip, port) == false)
        return false;
    return true;
}

void Socket::ReuseAddress()
{
    int val = 1;
    int n = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
    if (n == -1)
    {

        LOG_ERROR("reuse set error errno: %d errmsg: %s ", errno, strerror(errno));
    }
    val = 1;
    int m = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, (void *)&val, sizeof(int));
    if (m == -1)
    {
        LOG_ERROR("reuse set error errno: %d errmsg: %s ", errno, strerror(errno));
    }
}

void Socket::NonBlock()
{
    int flag = fcntl(_sockfd, F_GETFL, 0);
    fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
}
