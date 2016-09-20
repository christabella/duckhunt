#include "Player.hpp"
#include <cstdlib>
#include <iostream>

#define TIME_THRESHOLD 40
#define ROUND_THRESHOLD 2
#define PROBABILITY_THRESHOLD 0.5

namespace ducks {
Player::Player() : 
hmms(COUNT_SPECIES, HMM()),
stork_count(0)
{
}

Action Player::shoot(const GameState &pState, const Deadline &pDue)
{
    /*
     * Here you should write your clever algorithms to get the best action.
     * This skeleton never shoots.
     */

    return cDontShoot;

    int time = pState.getBird(0).getSeqLength();
    if (time < TIME_THRESHOLD && pState.getRound() < ROUND_THRESHOLD) {
        return cDontShoot;
    }

    MovePrediction movePrediction;
    double highest = 0;
    EMovement movement;
    Bird bird;
    int bird_idx;

    for (int i = 0; i < pState.getNumBirds(); i++) {
        bird = pState.getBird(i);

        if (bird.isDead())
            continue;

        movePrediction = predictMove(bird); // get highest prediction for each bird
        if (movePrediction.confidence > highest) {
            highest = movePrediction.confidence;
            movement = movePrediction.movement;
            bird_idx = i;
        }
        if (highest >= 0.55 && highest < 1 && stork_count > 1) {
            std::cerr << "SHOOTING AT BIRD | " << highest << "\n";
            return Action(bird_idx, movement);        
        }
    }

    return cDontShoot;

    //This line would predict that bird 0 will move right and shoot at it
    //return Action(0, MOVE_RIGHT);
}

std::vector<ESpecies> Player::guess(const GameState &pState, const Deadline &pDue)
{
    /*
     * Here you should write your clever algorithms to guess the species of each bird.
     * This skeleton makes no guesses, better safe than sorry!
     */

    int num_birds = pState.getNumBirds();
    std::vector<ESpecies> lGuesses(num_birds, SPECIES_UNKNOWN);
    // std::cerr << "--------------------Guesses--------------------" << std::endl;
    for (int i = 0; i < num_birds; ++i) {
        Bird bird = pState.getBird(i);
        lGuesses[i] = predictSpecies(bird);
        // std::cerr << lGuesses[i] << " | ";
    }
    // std::cerr << std::endl;
    return lGuesses;
}

void Player::hit(const GameState &pState, int pBird, const Deadline &pDue)
{
    /*
     * If you hit the bird you are trying to shoot, you will be notified through this function.
     */
    std::cerr << "HIT BIRD!!!" << std::endl;
}

void Player::reveal(const GameState &pState, const std::vector<ESpecies> &pSpecies, const Deadline &pDue)
{
    /*
     * If you made any guesses, you will find out the true species of those birds in this function.
     */
    // std::cerr << "--------------------Actual---------------------" << std::endl;

    for (int i = 0; i < pSpecies.size(); ++i) {
        int species = pSpecies[i];
        // std::cerr << species << " | ";
        if (pDue.remainingMs() < 80)
            break;
        // std::vector<EMovement> movements = getBirdMovements(pState.getBird(i));
        if (species == SPECIES_BLACK_STORK)
            stork_count += 1;
        // std::cerr << "gonna ESTIMATE MODEL\n";
        hmms[species].estimateModel(getBirdMovements(pState.getBird(i)));
        // std::cerr << "ESTIMATED MODEL\n";
        // std::cerr << "SPECIES: " << (ESpecies)species << std::endl;
        
    }
    // std::cerr << std::endl;

}

/**************************************** HELPER FUNCTIONS ****************************************/

std::vector<EMovement> Player::getBirdMovements(const Bird &bird) {
    std::vector<EMovement> movements;

    for (int i = 0; i < bird.getSeqLength(); i++) {
        movements.push_back(bird.getObservation(i));
    }

    return movements;
}

ESpecies Player::predictSpecies(const Bird &bird) {
    ESpecies species = SPECIES_PIGEON;
    std::vector<EMovement> movements = getBirdMovements(bird);
    double temp;
    double highest = 0;
    for (int i = 0; i < COUNT_SPECIES; ++i) {
        temp = hmms[i].probabilityOfObservationSequence(movements);
        // std::cerr << i << " SPECIES\n";
        // std::cerr << temp << " PROBABILITY\n ";
        if (temp > highest) {
            highest = temp;
            species = (ESpecies)i;
        }
    }
    // std::cerr << highest << std::endl;
    if (highest < 1.0e-300) {
        species = SPECIES_PIGEON;
    }
    return species;
}

MovePrediction Player::predictMove(const Bird &bird) {
    MovePrediction movePrediction;
    movePrediction.confidence = 0;
    double test_probability;
    std::vector<EMovement> movements = getBirdMovements(bird);
    std::vector<EMovement> test_movements = getBirdMovements(bird);

    for (int i = 0; i < COUNT_SPECIES; ++i) {
        for (int m = 0; m < COUNT_MOVE; ++m) {
            test_movements.push_back((EMovement)m);
            double test_probability = hmms[i].probabilityOfObservationSequence(test_movements);
            if (test_probability > movePrediction.confidence) {
                movePrediction.confidence = test_probability;
                movePrediction.movement = (EMovement)m;
            }
            test_movements.pop_back();
        }
    }
    return movePrediction;
}

} /*namespace ducks*/
