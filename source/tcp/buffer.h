#pragma once
#include <iostream>
#include <vector>
#include <cassert>
#include <string.h>
#include <cstdint>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 7;

public:
    Buffer();
    ~Buffer();

    // 返回初始化可读位置
    char *begin();

    char *ReadPosition();
    // 返回写位置
    char *WritePosition();

    // 可读元素个数
    uint64_t ReadAbleBytes();

    // 可写元素个数
    uint64_t WriteAbleBytes();

    // 头部空闲元素个数
    uint64_t HeadIdleBytes();

    // 尾部空闲元素个数
    uint64_t TailIdleBytes();

    void MoveReadOffset(uint64_t len);
    void MoveWriteOffset(uint64_t len);

    // 确保写空间足够
    void EnsureWriteSpace(uint64_t len);

    void Write(const void *data, uint64_t len);
    void WriteString(const std::string &str);
    void WriteBuffer(Buffer &data);

    void Read(char *buf, uint64_t len);
    std::string ReadAsString(uint64_t len);

    void Clear();

    char* FindCrlf(); // 找到换行
    std::string GetLine();

private:
    std::vector<char> buffer_;
    int read_idx_;
    int write_idx_;
};
