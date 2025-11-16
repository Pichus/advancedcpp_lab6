#include <array>
#include <iostream>
#include <optional>
#include <random>

#include "player.h"

struct GameState {
    int number1;
    int number2;
    int start_sum;
};

enum class Action {
    IncrementNumber = 0,
    MultiplyNumberBy2 = 1,
    DivideNumberBy3 = 2
};

bool TryApplyAction(int& target, const Action action) {
    switch (action) {
        case Action::IncrementNumber:
            target += 1;
            return true;
        case Action::MultiplyNumberBy2:
            target *= 2;
            return true;
        case Action::DivideNumberBy3:
            if (target % 3 != 0) {
                return false;
            }
            target /= 3;
            return true;
    }
    return false;
}

Player CreatePlayer(int player_id) {
    std::optional<Action> previous_action;
    std::mt19937 rng(std::random_device{}());

    while (true) {
        constexpr std::array<Action, 3> action_candidates{
            Action::IncrementNumber, Action::MultiplyNumberBy2,
            Action::DivideNumberBy3};

        std::vector<double> action_weights;
        action_weights.reserve(action_candidates.size());
        for (const Action action : action_candidates) {
            const bool action_is_new = !previous_action.has_value() ||
                                       action != previous_action.value();
            action_weights.push_back(action_is_new ? 1.0 : 0.0);
        }

        std::discrete_distribution<std::size_t> dist(action_weights.begin(),
                                                     action_weights.end());
        Action action = action_candidates[dist(rng)];

        const int target = std::uniform_int_distribution<int>(0, 1)(rng);

        co_yield Player::PromiseResultType{target, static_cast<int>(action),
                                           player_id};

        const Player::PromiseResponseType response =
            co_await Player::ResponseTag{};

        if (response == Player::PromiseResponseType::Win) {
            std::cout << "=== Player " << player_id << " WINS! ===\n";
            co_return {0, 0, player_id};
        }
        if (response == Player::PromiseResponseType::Invalid) {
            continue;
        }

        previous_action = action;
    }
}

int main() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 10);

    const int number1 = dist(rng);
    const int number2 = dist(rng);

    GameState game_state = {number1, number2, number1 + number2};

    std::array players = {CreatePlayer(1), CreatePlayer(2)};

    int current_player_id = 0;

    auto step = [&](const int id) -> bool {
        Player& current_player = players[id];

        const auto promise_result = current_player.current_value_safety();
        const int target = promise_result.target;
        const auto action = static_cast<Action>(promise_result.action_number);

        int& number_to_apply_action =
            (target == 0) ? game_state.number1 : game_state.number2;

        Player::PromiseResponseType response;

        if (!TryApplyAction(number_to_apply_action, action)) {
            response = Player::PromiseResponseType::Invalid;
        } else if (game_state.number1 + game_state.number2 >=
                   game_state.start_sum * 10) {
            response = Player::PromiseResponseType::Win;
        } else {
            response = Player::PromiseResponseType::Accepted;
        }

        current_player.set_response(response);
        current_player.move_next();

        if (response == Player::PromiseResponseType::Win) {
            std::cout << "Winner: Player " << (id + 1)
                      << " | Number 1 = " << game_state.number1
                      << " Number 2 = " << game_state.number2
                      << " sum=" << (game_state.number1 + game_state.number2)
                      << "\n";
            return true;
        }
        if (response == Player::PromiseResponseType::Accepted) {
            current_player_id ^= 1;
        }
        return false;
    };

    while (true) {
        if (step(current_player_id)) break;
    }

    return 0;
}
