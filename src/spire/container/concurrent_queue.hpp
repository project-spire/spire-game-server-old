#pragma once

#include <mutex>
#include <queue>

namespace spire {
template <typename T>
class ConcurrentQueue final {
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;
    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    void push(const T& item);
    void push(T&& item);
    T pop();
    void clear();

    bool empty() const;
    size_t size() const;

    void swap(std::queue<T>& other);

private:
    std::queue<T> _queue {};
    mutable std::mutex _mutex {};
};


template <typename T>
void ConcurrentQueue<T>::push(const T& item) {
    std::lock_guard lock {_mutex};

    _queue.push(item);
}

template <typename T>
void ConcurrentQueue<T>::push(T&& item) {
    std::lock_guard lock {_mutex};

    _queue.push(std::forward<T>(item));
}

template <typename T>
T ConcurrentQueue<T>::pop() {
    std::lock_guard lock {_mutex};

    T item {std::move(_queue.front())};
    _queue.pop();

    return item;
}

template <typename T>
void ConcurrentQueue<T>::clear() {
    std::lock_guard lock {_mutex};

    _queue = {};
}

template <typename T>
bool ConcurrentQueue<T>::empty() const {
    std::lock_guard lock {_mutex};

    return _queue.empty();
}

template <typename T>
size_t ConcurrentQueue<T>::size() const {
    std::lock_guard lock {_mutex};

    return _queue.size();
}

template <typename T>
void ConcurrentQueue<T>::swap(std::queue<T>& other) {
    std::lock_guard lock {_mutex};

    _queue.swap(other);
}
}
