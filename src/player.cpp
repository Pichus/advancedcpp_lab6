#include "player.h"

#include <array>
#include <iostream>
#include <random>

Player CreatePlayer(const int player_id, const int initial_number1,
                    const int initial_number2) {
    std::optional<Action> previous_action;
    std::mt19937 rng(std::random_device{}());

    int number1 = initial_number1;
    int number2 = initial_number2;

    while (true) {
        constexpr std::array action_candidates{Action::IncrementNumber,
                                               Action::MultiplyNumberBy2,
                                               Action::DivideNumberBy3};

        const int which_number = std::uniform_int_distribution<int>(0, 1)(rng);
        const int current_number = which_number == 0 ? number1 : number2;

        std::vector<double> action_weights;
        action_weights.reserve(action_candidates.size());

        for (const Action action : action_candidates) {
            const bool action_is_new = !previous_action.has_value() ||
                                       action != previous_action.value();
            const bool current_number_is_divisible_by_3 =
                current_number % 3 == 0;

            if (!action_is_new || (action == Action::DivideNumberBy3 &&
                                   !current_number_is_divisible_by_3)) {
                action_weights.push_back(0.0);
            } else {
                action_weights.push_back(1.0);
            }
        }

        std::discrete_distribution<std::size_t> dist(action_weights.begin(),
                                                     action_weights.end());
        Action action = action_candidates[dist(rng)];

        co_yield Player::PromiseResultType{which_number,
                                           static_cast<int>(action), player_id};

        const Player::PromiseResponseType response =
            co_await Player::ResponseTag{};

        number1 = response.number1;
        number2 = response.number2;

        if (response.status == Player::PromiseResponseStatus::Win) {
            std::cout << "=== Player " << player_id << " WINS! ===\n";
            co_return {0, 0, player_id};
        }

        previous_action = action;
    }
}