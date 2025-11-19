#include <array>
#include <iostream>
#include <optional>
#include <random>

#include "player.h"

struct GameState {
    int number1;
    int number2;
    int initial_sum;
};

int ApplyAction(const int value, const Action action) {
    switch (action) {
        case Action::IncrementNumber:
            return value + 1;
        case Action::MultiplyNumberBy2:
            return value * 2;
        case Action::DivideNumberBy3:
            return value / 3;
    }
    return value;
}

bool RunTurn(GameState& game_state, Player& current_player) {
    const auto promise_result = current_player.current_value_safety();
    const int target = promise_result.which_number;
    const auto action = static_cast<Action>(promise_result.action_number);

    const int number_to_apply_action =
        (target == 0) ? game_state.number1 : game_state.number2;

    const int apply_action_result = ApplyAction(number_to_apply_action, action);

    if (target == 0) {
        game_state.number1 = apply_action_result;
    } else {
        game_state.number2 = apply_action_result;
    }

    Player::PromiseResponseType response = {
        Player::PromiseResponseStatus::Accepted, game_state.number1,
        game_state.number2};

    if (game_state.number1 + game_state.number2 >=
        game_state.initial_sum * 10) {
        response.status = Player::PromiseResponseStatus::Win;
    }

    current_player.set_response(response);
    current_player.move_next();

    if (response.status == Player::PromiseResponseStatus::Win) {
        if (!promise_result.player_id.has_value()) {
            throw std::invalid_argument(
                "player id returned by the promise is null, "
                "should not happen");
        }

        std::cout << "Winner: Player " << (promise_result.player_id.value())
                  << " | Number 1 = " << game_state.number1
                  << " Number 2 = " << game_state.number2
                  << " sum=" << (game_state.number1 + game_state.number2)
                  << "\n";
        return true;
    }

    return false;
}

void StartGameEngine() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 10);

    const int number1 = dist(rng);
    const int number2 = dist(rng);

    GameState game_state = {number1, number2, number1 + number2};

    std::array players = {CreatePlayer(1, number1, number2),
                          CreatePlayer(2, number1, number2)};

    int current_player_id = 0;

    while (true) {
        bool turn_result = RunTurn(game_state, players[current_player_id]);
        if (turn_result) break;
        current_player_id ^= 1;
    }
}

int main() {
    try {
        StartGameEngine();
    } catch (std::invalid_argument& exception) {
        std::cout << exception.what() << std::endl;
    }
    return 0;
}
