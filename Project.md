## Project: "RingBuffer" — a fixed-size circular buffer in C++

A header-only C++ ring buffer with a small test suite and a tiny demo. The thesis: _"I can write correct, memory-safe C++ with an understanding of manual memory and fixed-size data structures."_ It's deliberately small — one data structure done well beats a sprawling app that hides weak fundamentals.

Why a ring buffer specifically: it's a canonical systems-C++ artifact. It forces you to deal with the exact things a C/C++ interviewer probes — manual memory, wraparound index arithmetic, the empty-vs-full ambiguity, and (if you go further) the rule of five and templates. And it's thematically tied to SensorDeck: a ring buffer is _exactly_ what you'd use to hold a rolling window of incoming telemetry, so your portfolio tells one coherent "hardware-facing software" story.

This maps to their bullets like this:

| Their requirement                  | How the project covers it                                    |
| ---------------------------------- | ------------------------------------------------------------ |
| C/C++ required                     | The whole thing is modern C++                                |
| OOP, design patterns               | A well-encapsulated class; RAII for memory ownership         |
| Unit + integration testing         | Test suite covering the tricky edge cases                    |
| Communication protocols / hardware | Framed as a telemetry buffer; pairs with SensorDeck's stream |
| Software architecture              | Clean API design, ownership semantics, generic via templates |

## Architecture

**1. The buffer class (`ring_buffer.hpp`)**
A template `RingBuffer<T>` so it works for bytes, telemetry frames, whatever. Core design decisions you'll make (and be able to defend):

- **Storage:** a fixed-capacity array. Decide early: `std::unique_ptr<T[]>` allocated once in the constructor (shows RAII + manual memory) vs. `std::array` with compile-time size. The `unique_ptr` route is the better interview signal because it demonstrates ownership without leaking — go that way.
- **Indices:** `head_` (write), `tail_` (read), and a `size_` or full-flag. The classic trap is empty-vs-full: when `head == tail`, is the buffer empty or full? Pick a strategy (tracking `size_` explicitly is the simplest to reason about and explain) and know why.
- **Overwrite policy:** on `push` when full, do you overwrite the oldest element or reject the write? For a telemetry buffer, overwrite-oldest is the natural choice — a rolling window. Make it a deliberate decision you can articulate.

**2. Public API** — keep it tight:

- `push(const T&)` / `push(T&&)` — add an element (overwrite oldest if full)
- `pop() -> std::optional<T>` — remove and return oldest, or `nullopt` if empty
- `front()` / `back()`, `size()`, `capacity()`, `empty()`, `full()`, `clear()`

**3. Rule of five** — this is where you show real C++ competency. Because you own a raw allocation via `unique_ptr`, think through copy constructor, copy assignment, move constructor, move assignment, destructor. Using `unique_ptr` gets you correct move semantics and a correct destructor nearly for free, and you can `= delete` or implement copy deliberately. Being able to explain _why the rule of five matters here_ is a top-tier interview answer for an entry C++ role.

**4. Tiny demo (`main.cpp`)** — push more items than capacity, show it rotates correctly, pop them back out in order. Proves the wraparound visually.

## What "done" looks like

`ring_buffer.hpp` you could hand to someone, a `main.cpp` that visibly demonstrates wraparound and overwrite, and a green test suite covering the edge cases below. Small enough to read in five minutes, correct enough to survive scrutiny.

## Testing (the part that proves correctness — don't skip)

Use **Catch2** or **GoogleTest** (Catch2 is header-only, less setup — matches the "keep it small" goal). The tests _are_ the credibility here, because they show you know where circular buffers break:

- push/pop basic FIFO ordering
- **fill exactly to capacity**, then one more — assert oldest was overwritten
- **empty buffer:** `pop()` returns `nullopt`, `front()` handled safely
- **wraparound:** push past the end so `head_` wraps to 0, assert order still correct
- **full → drain → refill** cycle
- move semantics: move-construct a buffer, assert source is valid-but-empty and contents transferred
- (optional) a non-trivial type (e.g. a struct or `std::string`) to prove templating and correct construction/destruction of elements

## Suggested build order

1. Non-generic `RingBuffer` of `int`, fixed capacity, `push`/`pop`/`size` only. Get FIFO working.
2. Add the empty/full logic and the overwrite-oldest policy; test wraparound.
3. Templatize to `RingBuffer<T>`.
4. Add rule-of-five members via `unique_ptr` storage; test move semantics.
5. Add `std::optional` returns, `front`/`back`/`clear`, tidy the API.
6. Write the full test suite; add the demo `main.cpp`.

Each step compiles and runs, same checkpoint style as SensorDeck.

## Build setup

Keep the toolchain minimal so this doesn't become the second-toolchain burden Claude Code warned about: a single `CMakeLists.txt` pulling in Catch2, building the demo and the tests. On Windows this drops straight into Visual Studio (it opens CMake projects natively), so you stay in the same environment as SensorDeck — no separate tooling to learn.
