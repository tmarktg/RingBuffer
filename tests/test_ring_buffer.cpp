#include <catch2/catch_test_macros.hpp>
#include <string>

#include "ring_buffer.hpp"

TEST_CASE("basic FIFO push/pop ordering", "[ring_buffer]") {
    RingBuffer<int> buf(3);
    buf.push(1);
    buf.push(2);
    buf.push(3);

    REQUIRE(buf.size() == 3);
    REQUIRE(*buf.pop() == 1);
    REQUIRE(*buf.pop() == 2);
    REQUIRE(*buf.pop() == 3);
    REQUIRE(buf.empty());
}

TEST_CASE("fill exactly to capacity, then one more overwrites oldest", "[ring_buffer]") {
    RingBuffer<int> buf(3);
    buf.push(1);
    buf.push(2);
    buf.push(3);
    REQUIRE(buf.full());

    buf.push(4);  // overwrites 1
    REQUIRE(buf.size() == 3);
    REQUIRE(*buf.pop() == 2);
    REQUIRE(*buf.pop() == 3);
    REQUIRE(*buf.pop() == 4);
}

TEST_CASE("empty buffer returns nullopt/nullptr safely", "[ring_buffer]") {
    RingBuffer<int> buf(3);
    REQUIRE(buf.empty());
    REQUIRE(buf.pop() == std::nullopt);
    REQUIRE(buf.front() == nullptr);
    REQUIRE(buf.back() == nullptr);
}

TEST_CASE("wraparound: head_ wraps to 0, order preserved", "[ring_buffer]") {
    RingBuffer<int> buf(3);
    buf.push(1);
    buf.push(2);
    buf.pop();          // tail_ advances, size_ == 1
    buf.push(3);
    buf.push(4);        // head_ wraps around to slot 0
    REQUIRE(buf.full());

    REQUIRE(*buf.pop() == 2);
    REQUIRE(*buf.pop() == 3);
    REQUIRE(*buf.pop() == 4);
    REQUIRE(buf.empty());
}

TEST_CASE("full -> drain -> refill cycle", "[ring_buffer]") {
    RingBuffer<int> buf(2);
    buf.push(1);
    buf.push(2);
    REQUIRE(buf.full());

    REQUIRE(*buf.pop() == 1);
    REQUIRE(*buf.pop() == 2);
    REQUIRE(buf.empty());

    buf.push(10);
    buf.push(20);
    REQUIRE(buf.full());
    REQUIRE(*buf.pop() == 10);
    REQUIRE(*buf.pop() == 20);
}

TEST_CASE("move construction leaves source valid-but-empty", "[ring_buffer]") {
    RingBuffer<int> src(3);
    src.push(1);
    src.push(2);

    RingBuffer<int> dst(std::move(src));
    REQUIRE(dst.size() == 2);
    REQUIRE(*dst.pop() == 1);
    REQUIRE(*dst.pop() == 2);

    REQUIRE(src.empty());
    REQUIRE(src.capacity() == 0);
    REQUIRE(src.pop() == std::nullopt);
}

TEST_CASE("move assignment leaves source valid-but-empty", "[ring_buffer]") {
    RingBuffer<int> src(3);
    src.push(1);
    src.push(2);

    RingBuffer<int> dst(1);
    dst.push(99);
    dst = std::move(src);

    REQUIRE(dst.size() == 2);
    REQUIRE(*dst.pop() == 1);
    REQUIRE(*dst.pop() == 2);

    REQUIRE(src.empty());
    REQUIRE(src.capacity() == 0);
}

TEST_CASE("copy construction/assignment produce independent buffers", "[ring_buffer]") {
    RingBuffer<int> original(3);
    original.push(1);
    original.push(2);

    RingBuffer<int> copy(original);
    copy.push(3);          // fills copy, shouldn't affect original
    REQUIRE(original.size() == 2);
    REQUIRE(copy.size() == 3);

    RingBuffer<int> assigned(1);
    assigned = original;
    REQUIRE(assigned.size() == 2);
    REQUIRE(*assigned.pop() == 1);
    REQUIRE(*assigned.pop() == 2);
    REQUIRE(original.size() == 2);  // untouched by popping the copy
}

TEST_CASE("non-trivial type: std::string constructs/destructs correctly", "[ring_buffer]") {
    RingBuffer<std::string> buf(2);
    buf.push(std::string("hello"));
    buf.push(std::string("world"));
    buf.push(std::string("overwrite"));  // drops "hello"

    REQUIRE(buf.size() == 2);
    REQUIRE(*buf.pop() == "world");
    REQUIRE(*buf.pop() == "overwrite");
    REQUIRE(buf.empty());
}

TEST_CASE("clear resets buffer to empty", "[ring_buffer]") {
    RingBuffer<int> buf(3);
    buf.push(1);
    buf.push(2);
    buf.clear();
    REQUIRE(buf.empty());
    REQUIRE(buf.size() == 0);
    REQUIRE(buf.pop() == std::nullopt);
}

TEST_CASE("front/back reflect oldest and newest elements", "[ring_buffer]") {
    RingBuffer<int> buf(3);
    buf.push(1);
    buf.push(2);
    REQUIRE(*buf.front() == 1);
    REQUIRE(*buf.back() == 2);

    buf.push(3);
    buf.push(4);  // overwrites 1
    REQUIRE(*buf.front() == 2);
    REQUIRE(*buf.back() == 4);
}
