#ifndef ADVANCEDCPP_LAB6_PLAYER_H
#define ADVANCEDCPP_LAB6_PLAYER_H

#include <coroutine>
#include <optional>
#include <stdexcept>
#include <string>

class Player {
   public:
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;

    struct PromiseResultType {
        int which_number;
        int action_number;
        std::optional<int> player_id;
    };

    struct ResponseTag {};

    enum class PromiseResponseStatus { Invalid, Accepted, Win };

    struct PromiseResponseType {
        PromiseResponseStatus status;
        int number1;
        int number2;
    };

    bool in_progress() const { return coro && !coro.done(); }

    bool move_next() {
        return in_progress() ? (coro.resume(), !coro.done()) : false;
    }

    PromiseResultType current_value_safety() const {
        if (!coro) throw std::runtime_error("coroutine is destroyed");
        return coro.promise().result;
    }

    void set_response(PromiseResponseType new_response) {
        if (!coro) throw std::runtime_error("coroutine is destroyed");
        coro.promise().response = new_response;
    }

    operator bool() const noexcept { return static_cast<bool>(coro); }

    class Awaiter {
       public:
        explicit Awaiter(promise_type& p) : p(p) {}
        bool await_ready() const noexcept { return true; }
        bool await_suspend(std::coroutine_handle<>) noexcept { return false; }
        PromiseResponseType await_resume() noexcept { return p.response; }

       private:
        promise_type& p;
    };

    struct promise_type {
        PromiseResultType result{};
        PromiseResponseType response{};

        void unhandled_exception() { std::terminate(); }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto yield_value(PromiseResultType value) {
            result = value;
            return std::suspend_always{};
        }

        void return_value(PromiseResultType value) { result = value; }

        auto get_return_object() { return Player{handle::from_promise(*this)}; }

        Awaiter await_transform(ResponseTag) { return Awaiter{*this}; }
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

enum class Action {
    IncrementNumber = 0,
    MultiplyNumberBy2 = 1,
    DivideNumberBy3 = 2
};

Player CreatePlayer(int player_id, int initial_number1, int initial_number2);

#endif  // ADVANCEDCPP_LAB6_PLAYER_H
