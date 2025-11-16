#ifndef ADVANCEDCPP_LAB6_PLAYER_H
#define ADVANCEDCPP_LAB6_PLAYER_H

#include <coroutine>
#include <stdexcept>
#include <string>

class Player {
   public:
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;

    bool in_progress() const { return coro && !coro.done(); }

    bool move_next() {
        return in_progress() ? (coro.resume(), !coro.done()) : false;
    }

    int current_value_safety() const {
        if (!coro) throw std::runtime_error("coroutine is destroyed");
        return coro.promise().result;
    }

    void set_response(int r) {
        if (!coro) throw std::runtime_error("coroutine is destroyed");
        coro.promise().response = r;
    }

    operator bool() const noexcept { return static_cast<bool>(coro); }

    class Awaiter {
       public:
        explicit Awaiter(promise_type& p) : p(p) {}
        bool await_ready() const noexcept { return true; }
        bool await_suspend(std::coroutine_handle<>) noexcept { return false; }
        int await_resume() noexcept { return p.response; }

       private:
        promise_type& p;
    };

    struct promise_type {
        int result = 0;
        int response = 0;

        void unhandled_exception() { std::terminate(); }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto yield_value(int value) {
            result = value;
            return std::suspend_always{};
        }

        void return_value(int value) { result = value; }

        auto get_return_object() { return Player{handle::from_promise(*this)}; }

        Awaiter await_transform(int) { return Awaiter{*this}; }
    };

    Player(Player const&) = delete;
    Player& operator=(Player const&) = delete;

    Player(Player&& other) noexcept : coro(other.coro) { other.coro = nullptr; }
    Player& operator=(Player&& other) noexcept {
        if (this != &other) {
            if (coro) coro.destroy();
            coro = other.coro;
            other.coro = nullptr;
        }
        return *this;
    }

    ~Player() {
        if (coro) coro.destroy();
    }

   private:
    explicit Player(handle h) : coro(h) {}
    handle coro = nullptr;
};

Player play_guess_the_number(const std::string& question);

#endif  // ADVANCEDCPP_LAB6_PLAYER_H
