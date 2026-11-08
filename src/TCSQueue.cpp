#include "TCSQueue.h"
#include "export.h"
#include <stdexcept>
#include <cstdlib>
#include <cstring>
TCSQueue::TCSQueue(const char* type) {
    char value[1024] = {};
    if (GetConfig(const_cast<char*>(type), const_cast<char*>("MAX_DEEPTH"), value) == 0) depth_ = std::strtol(value, nullptr, 10);
    if (GetConfig(const_cast<char*>(type), const_cast<char*>("MAX_UNIT_SIZE"), value) == 0) unit_ = std::strtol(value, nullptr, 10);
    if (depth_ < 1 || depth_ > 100000 || unit_ < 1 || unit_ > 16777216) throw std::runtime_error("Invalid queue limits");
}
int TCSQueue::Put(long n, char* data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!data || n < 0 || n > unit_ || queue_.size() >= static_cast<size_t>(depth_)) return -1;
    queue_.emplace_back(data, data + n); return 0;
}
int TCSQueue::Get(long cap, char* data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!data || cap < 0 || queue_.empty() || queue_.front().size() > static_cast<size_t>(cap)) return -1;
    std::memset(data, 0, cap); std::memcpy(data, queue_.front().data(), queue_.front().size());
    queue_.pop_front(); return 0;
}
int TCSQueue::Peek(long cap, char* data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!data || cap < 0 || queue_.empty() || queue_.front().size() > static_cast<size_t>(cap)) return -1;
    std::memset(data, 0, cap); std::memcpy(data, queue_.front().data(), queue_.front().size()); return 0;
}
int TCSQueue::Count() { std::lock_guard<std::mutex> lock(mutex_); return static_cast<int>(queue_.size()); }
void TCSQueue::Clear() { std::lock_guard<std::mutex> lock(mutex_); queue_.clear(); }
long TCSQueue::GetMaxDeepth() { return depth_; }
long TCSQueue::GetMaxUnitSize() { return unit_; }
long TCSQueue::GetNextSize() { std::lock_guard<std::mutex> lock(mutex_); return queue_.empty() ? 0 : static_cast<long>(queue_.front().size()); }
