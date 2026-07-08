#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity)
        : buf_(std::make_unique<T[]>(capacity)), capacity_(capacity) {}

    RingBuffer(const RingBuffer& other)
        : buf_(std::make_unique<T[]>(other.capacity_)),
          capacity_(other.capacity_),
          head_(other.head_),
          tail_(other.tail_),
          size_(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) {
            std::size_t idx = (other.tail_ + i) % other.capacity_;
            buf_[idx] = other.buf_[idx];
        }
    }

    RingBuffer& operator=(const RingBuffer& other) {
        if (this == &other) return *this;
        auto new_buf = std::make_unique<T[]>(other.capacity_);
        for (std::size_t i = 0; i < other.size_; ++i) {
            std::size_t idx = (other.tail_ + i) % other.capacity_;
            new_buf[idx] = other.buf_[idx];
        }
        buf_ = std::move(new_buf);
        capacity_ = other.capacity_;
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        return *this;
    }

    // Moved-from buffer is left valid-but-empty (size_ == 0, capacity_ == 0).
    RingBuffer(RingBuffer&& other) noexcept
        : buf_(std::move(other.buf_)),
          capacity_(other.capacity_),
          head_(other.head_),
          tail_(other.tail_),
          size_(other.size_) {
        other.capacity_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
        other.size_ = 0;
    }

    RingBuffer& operator=(RingBuffer&& other) noexcept {
        if (this == &other) return *this;
        buf_ = std::move(other.buf_);
        capacity_ = other.capacity_;
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        other.capacity_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
        other.size_ = 0;
        return *this;
    }

    ~RingBuffer() = default;

    void push(const T& value) { push_impl(value); }
    void push(T&& value) { push_impl(std::move(value)); }

    std::optional<T> pop() {
        if (empty()) return std::nullopt;
        T value = std::move(buf_[tail_]);
        tail_ = advance(tail_);
        --size_;
        return value;
    }

    T* front() { return empty() ? nullptr : &buf_[tail_]; }
    const T* front() const { return empty() ? nullptr : &buf_[tail_]; }

    T* back() {
        if (empty()) return nullptr;
        std::size_t last = (head_ == 0 ? capacity_ : head_) - 1;
        return &buf_[last];
    }
    const T* back() const {
        if (empty()) return nullptr;
        std::size_t last = (head_ == 0 ? capacity_ : head_) - 1;
        return &buf_[last];
    }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == capacity_; }

    void clear() {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

private:
    std::size_t advance(std::size_t idx) const { return (idx + 1) % capacity_; }

    template <typename U>
    void push_impl(U&& value) {
        buf_[head_] = std::forward<U>(value);
        head_ = advance(head_);
        if (full()) {
            // Overwrite-oldest policy: a rolling telemetry window, not a
            // reject-on-full buffer, so the reader advances too.
            tail_ = advance(tail_);
        } else {
            ++size_;
        }
    }

    std::unique_ptr<T[]> buf_;
    std::size_t capacity_ = 0;
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
};
