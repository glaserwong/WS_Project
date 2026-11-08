#ifndef INCLUDED_TCSQUEUE_H
#define INCLUDED_TCSQUEUE_H
#include <deque>
#include <vector>
#include <mutex>
#define SECS_QUEUE "SECS_QUEUE"
#define TRX_QUEUE "TRX_QUEUE"
class TCSQueue {
public:
    explicit TCSQueue(const char* type);
    virtual ~TCSQueue() = default;
    int Put(long length, char* data);
    int Get(long capacity, char* data);
    int Peek(long capacity, char* data);
    int Count();
    void Clear();
    long GetMaxDeepth();
    long GetMaxUnitSize();
    long GetNextSize();
private:
    std::mutex mutex_;
    std::deque<std::vector<char>> queue_;
    long depth_ = 5, unit_ = 1000;
};
#endif
