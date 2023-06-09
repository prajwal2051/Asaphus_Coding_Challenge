/**
 * @file asaphus_coding_challenge.cpp
 * @version 1.1
 * @copyright Copyright (c) 2022 Asaphus Vision GmbH
 *
 * The goal is to implement the mechanics of a simple game and write test cases for them.
 * The rules of the game are:
 * - There are two types of boxes, green and blue. 
 * - Both can absorb tokens of a given weight, which they add to their own total weight. 
 * - Both are initialized with a given initial weight.
 * - After a box absorbs a token weight, it outputs a score. 
 * - Green and blue boxes calculate the score in different ways:
 * - A green box calculates the score as the square of the mean of the 3 weights that it most recently absorbed (square of mean of all absorbed weights if there are fewer than 3).
 * - A blue box calculates the score as Cantor's pairing function of the smallest and largest weight that it has absorbed so far, i.e. pairing(smallest, largest), where pairing(0, 1) = 2
 * - The game is played with two green boxes with initial weights 0.0 and 0.1, and two blue boxes with initial weights 0.2 and 0.3.
 * - There is a list of input token weights. Each gets used in one turn.
 * - There are two players, A and B. Both start with a score of 0. 
 * - The players take turns alternatingly. Player A starts.
 * - In each turn, the current player selects one of the boxes with the currently smallest weight, and lets it absorb the next input token weight. Each input weight gets only used once.
 * - The result of the absorption gets added to the current player's score.
 * - When all input token weights have been used up, the game ends, and the player with highest score wins.
 *
 * Task:
 * - Create a git repo for the implementation. Use the git repository to have a commit history.
 * - Implement all missing parts, marked with "TODO", including the test cases.
 * - Split the work in reasonable commits (not just one commit).
 * - Make sure the test cases succeed.
 * - Produce clean, readable code.
 *
 * Notes:
 * - Building and running the executable: g++ --std=c++14 asaphus_coding_challenge.cpp -o challenge && ./challenge
 * - Feel free to add a build system like CMake, meson, etc.
 * - Feel free to add more test cases, if you would like to test more.
 * - This file includes the header-only test framework Catch v2.13.9.
 * - A main function is not required, as it is provided by the test framework.
 */

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <numeric>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"


/**
 * Base class representing a Box.
 */
class Box {
public:
    explicit Box(double initial_weight) : weight_(initial_weight) {}

    virtual ~Box() = default;

    static std::unique_ptr<Box> makeGreenBox(double initial_weight);

    static std::unique_ptr<Box> makeBlueBox(double initial_weight);

    bool operator<(const Box& rhs) const { return weight_ < rhs.weight_; }

    /**
     * Absorbs the given weight into the Box.
     */
    virtual double absorb(double weight) {
        weight_ += weight;
        return 0;
    }

    /**
     * Retrieves the weight of the Box.
     */

    double getWeight() const {
        return weight_;
    }

private:
    /**
     * Calculates the score of the Box.
     *
     */
    virtual double calculateScore() const = 0;

 protected:
  double weight_;
};

/**
 * Calculates the mean value of a vector of doubles.
 *
 */
double mean(const std::vector<double>& a) {
    if (a.size() == 0) return 0;
    double sum = 0;
    for (auto x : a) {
        sum += x;
    }
    return sum / a.size();
}


/**
 * Performs the Cantor Pairing function on two input values.
 *
 */
double cantorPairing(double x, double y) {
    return (x + y) * (x + y + 1) / 2 + y;
}

/**
 * Derived class representing a GreenBox
 */

class GreenBox : public Box {
public:
    std::vector<double> recentWeights;

    explicit GreenBox(double initial_weight) : Box(initial_weight) {};

    double absorb(double weight) override {
        Box::absorb(weight);
        if (recentWeights.size() == 3) {
            recentWeights[0] = recentWeights[1];
            recentWeights[1] = recentWeights[2];
            recentWeights[2] = weight;
        }
        else {
            recentWeights.push_back(weight);
        }
        return calculateScore();
    }

private:
    double calculateScore() const override {
        double m = mean(recentWeights);
        return m * m;
    };
};

/**
 * Derived class representing a BlueBox
 */
class BlueBox : public Box {
public:

    explicit BlueBox(double initial_weight) : Box(initial_weight), absorbedAtLeastOneWeight(false) {};

    double absorb(double weight) override {
        Box::absorb(weight);
        if (absorbedAtLeastOneWeight) {
            minWeight = std::min(minWeight, weight);
            maxWeight = std::max(maxWeight, weight);
        }
        else {
            minWeight = maxWeight = weight;
        }
        absorbedAtLeastOneWeight = true;
        return calculateScore();
    }

private:
    double minWeight, maxWeight;
    bool absorbedAtLeastOneWeight;

    double calculateScore() const override {
        return cantorPairing(minWeight, maxWeight);
    }
};

// Factory method to create a green box with the given initial weight
std::unique_ptr<Box> Box::makeGreenBox(double initial_weight) {
    return std::make_unique<GreenBox>(initial_weight);
}

// Factory method to create a blue box with the given initial weight
std::unique_ptr<Box> Box::makeBlueBox(double initial_weight) {
    return std::make_unique<BlueBox>(initial_weight);
}

/**
 * Class representing a Player.
 */
class Player {
public:
    void takeTurn(uint32_t input_weight,
        const std::vector<std::unique_ptr<Box> >& boxes) {
        /**
         * Find the box with the smallest weight
         */
        auto smallestWeightBox = boxes[0].get();
        for (auto& box : boxes) {
            if (*box < *smallestWeightBox) {
                smallestWeightBox = box.get();
            }
        }
        score_ += smallestWeightBox->absorb(input_weight);
    }

    double getScore() const { return score_; }

private:
    double score_{ 0.0 };
};

/**
 * Plays the game with the given input weights.
 */
std::pair<double, double> play(const std::vector<uint32_t>& input_weights) {
    std::vector<std::unique_ptr<Box> > boxes;
    boxes.emplace_back(Box::makeGreenBox(0.0));
    boxes.emplace_back(Box::makeGreenBox(0.1));
    boxes.emplace_back(Box::makeBlueBox(0.2));
    boxes.emplace_back(Box::makeBlueBox(0.3));

    Player player_A;
    Player player_B;
    int turn = 0;
    for (auto weight : input_weights) {
        if (turn == 0) {
            player_A.takeTurn(weight, boxes);
        }
        else {
            player_B.takeTurn(weight, boxes);
        }
        turn = (turn + 1) % 2;
    }

    std::cout << "Scores: player A " << player_A.getScore() << ", player B "
        << player_B.getScore() << std::endl;
    return std::make_pair(player_A.getScore(), player_B.getScore());
}

// Test cases

TEST_CASE("Final scores for first 4 Fibonacci numbers", "[fibonacci4]") {
    std::vector<uint32_t> inputs{ 1, 1, 2, 3 };
    auto result = play(inputs);
    REQUIRE(result.first == 13.0);
    REQUIRE(result.second == 25.0);
}

TEST_CASE("Final scores for first 8 Fibonacci numbers", "[fibonacci8]") {
    std::vector<uint32_t> inputs{ 1, 1, 2, 3, 5, 8, 13, 21 };
    auto result = play(inputs);
    REQUIRE(result.first == 155.0);
    REQUIRE(result.second == 366.25);
}

TEST_CASE("Test absorption of green box", "[green]") {
    std::unique_ptr<Box> greenBox = Box::makeGreenBox(1.0);
    REQUIRE(greenBox->absorb(1.0) == 1.0);
    REQUIRE(greenBox->absorb(2.0) == 1.5 * 1.5);
    REQUIRE(greenBox->absorb(3.0) == 2.0 * 2.0);
    REQUIRE(greenBox->absorb(4.0) == 3.0 * 3.0);
    REQUIRE(greenBox->getWeight() == 11.0);
}

TEST_CASE("Test absorption of blue box", "[blue]") {
    std::unique_ptr<Box> blueBox = Box::makeBlueBox(1.0);
    REQUIRE(blueBox->absorb(3.0) == 24.0);
    REQUIRE(blueBox->absorb(2.0) == 18.0);
    REQUIRE(blueBox->absorb(4.0) == 25.0);
    REQUIRE(blueBox->absorb(1.0) == 19.0);
    REQUIRE(blueBox->getWeight() == 11.0);

}


/**
* Final Output in the console Window
Scores: player A 13, player B 25
Scores: player A 155, player B 366.25
===============================================================================
All tests passed (14 assertions in 4 test cases)
*/