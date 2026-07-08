#include <iostream>

#include "ring_buffer.hpp"

int main() {
    RingBuffer<int> buf(4);

    std::cout << "Pushing 1..6 into a capacity-4 buffer (expect overwrite):\n";
    for (int i = 1; i <= 6; ++i) {
        buf.push(i);
        std::cout << "  pushed " << i << " -> [";
        // Peek without mutating: pop everything into a temp buffer, print, push back.
        RingBuffer<int> snapshot = buf;
        bool first = true;
        while (auto v = snapshot.pop()) {
            if (!first) std::cout << ", ";
            std::cout << *v;
            first = false;
        }
        std::cout << "] size=" << buf.size() << "\n";
    }

    std::cout << "\nDraining buffer in FIFO order:\n";
    while (auto v = buf.pop()) {
        std::cout << "  popped " << *v << "\n";
    }

    std::cout << "buffer empty after drain: " << std::boolalpha << buf.empty() << "\n";
    return 0;
}
