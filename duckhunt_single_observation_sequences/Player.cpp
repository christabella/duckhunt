#include "Player.hpp"
#include <cstdlib>
#include <iostream>

#define TIME_THRESHOLD 40
#define ROUND_THRESHOLD 2
#define HMMS_THRESHOLD 40
#define PROBABILITY_THRESHOLD 0.5

namespace ducks {
Player::Player() : 
hmms(COUNT_SPECIES, std::vector<HMM>()),
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

    // int time = pState.getBird(0).getSeqLength();
    // if (time < TIME_THRESHOLD && pState.getRound() < ROUND_THRESHOLD) {
    //     return cDontShoot;
    // }

    MovePrediction movePrediction;
    double highest = 0;
    EMovement movement;
    Bird bird;
    int bird_idx;

    for (int i = 0; i < pState.getNumBirds(); ++i) {
        bird = pState.getBird(i);

        if (bird.isDead()) {
            continue;
        }

        movePrediction = predictMove(bird); // get highest prediction for each bird
        if (movePrediction.confidence > highest) {
            highest = movePrediction.confidence;
            movement = movePrediction.movement;
            bird_idx = i;
        }
        // std::cerr << "\nhighest: " << highest;
        if (highest >= 1.0e-9 && highest < 1) {
            if (stork_count < 2) {
                std::cerr << "SHOOTING AT BIRD | " << highest << "\n";
                return Action(bird_idx, movement);        
                
            } else if (predictSpecies(bird) != SPECIES_BLACK_STORK) {
                std::cerr << "SHOOTING AT BIRD | " << highest << "\n";
                return Action(bird_idx, movement);        

            } else {
                continue;
            }
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
    int len = pSpecies.size();

    for (int i = 0; i < len; ++i) {
        int species = pSpecies[i];
        // std::cerr << species << " | ";
        // if (pDue.remainingMs() < 80)
        //     break;
        // std::vector<EMovement> movements = getBirdMovements(pState.getBird(i));
        if (species == SPECIES_BLACK_STORK)
            stork_count += 1;
        hmms[species].push_back(HMM(getBirdMovements(pState.getBird(i))));
        // std::cerr << "SPECIES: " << (ESpecies)species << std::endl;
        
    }
    // std::cerr << std::endl;

}

/**************************************** HELPER FUNCTIONS ****************************************/

std::vector<EMovement> Player::getBirdMovements(const Bird &bird) {
    int len = bird.getSeqLength();

    std::vector<EMovement> movements(len);

    for (int i = 0; i < len; ++i) {
        movements[i] = bird.getObservation(i);
    }

    return movements;
}

ESpecies Player::predictSpecies(const Bird &bird) {
    ESpecies species = SPECIES_PIGEON;
    std::vector<EMovement> movements = getBirdMovements(bird);
    double prob_species;
    int hmms_size;
    double highest = 0;
    for (int i = 0; i < COUNT_SPECIES; ++i) {
            // std::cerr << i << "FUCKITYFUCK";
        prob_species = 0;
        hmms_size = hmms[i].size();
        if (hmms_size > 1) {
            for (int j = 0; j < hmms_size ; ++j) {
                prob_species += hmms[i][j].probabilityOfObservationSequence(movements);
                // std::cerr << prob_species << " | ";
            }
            prob_species /= hmms_size;
            if (prob_species > highest) {
                highest = prob_species;
                species = (ESpecies)i;
            }
        }
    }
    // std::cerr << highest << std::endl;
    if (highest < 1.0e-320) {
        species = SPECIES_PIGEON;
    }
    return species;
}

MovePrediction Player::predictMove(const Bird &bird) {
    MovePrediction movePrediction;
    movePrediction.confidence = 0;
    int hmms_size;
    double prob_hmms;
    std::vector<EMovement> movements = getBirdMovements(bird);
    std::vector<EMovement> test_movements = getBirdMovements(bird);

    for (int m = 0; m < 9; ++m) {
        if (m == 5)
            continue;
        test_movements.push_back((EMovement)m);
        for (int i = 0; i < COUNT_SPECIES; ++i) {
            hmms_size = hmms[i].size();
            if (hmms_size > HMMS_THRESHOLD) {
                prob_hmms = 0;
                for (int j = 0; j < hmms_size; ++j) {
                    prob_hmms += hmms[i][j].probabilityOfObservationSequence(test_movements);
                    // std::cerr << "\n PROB_HMM: " << hmms[i][j].probabilityOfObservationSequence(test_movements);
                }

                // std::cerr << "\n HMMS SIZE: " << hmms_size;
                // std::cerr << "\n PROB_HMMSSSS: " << prob_hmms;
                prob_hmms /= hmms_size;
                if (prob_hmms > movePrediction.confidence) {
                    movePrediction.confidence = prob_hmms;
                    movePrediction.movement = (EMovement)m;
                }
                
            }
        }
        // std::cerr << "\n PROB_MOVE: " << prob_move;
        // std::cerr << "\n PROB_MOVE: " << prob_move;
        test_movements.pop_back();
    }
    return movePrediction;
}

/***** getting mean of all probabilities from same move, diff species *****/
// MovePrediction Player::predictMove(const Bird &bird) {
//     MovePrediction movePrediction;
//     movePrediction.confidence = 0;
//     double hmms_size;
//     double prob_hmms;
//     double prob_move;
//     std::vector<EMovement> movements = getBirdMovements(bird);
//     std::vector<EMovement> test_movements = getBirdMovements(bird);

//     for (int m = 0; m < 9; ++m) {
//         if (m == 5)
//             continue;
//         test_movements.push_back((EMovement)m);
//         prob_move = 0;
//         for (int i = 0; i < COUNT_SPECIES; ++i) {
//             prob_hmms = 0;
//             hmms_size = hmms[i].size();

//             for (int j = 0; j < hmms_size; ++j) {
//                 prob_hmms += hmms[i][j].probabilityOfObservationSequence(test_movements);
//                 // std::cerr << "\n PROB_HMM: " << hmms[i][j].probabilityOfObservationSequence(test_movements);
//             }

//             if (hmms_size != 0)
//                 prob_hmms /= hmms_size;
//             // std::cerr << "\n HMMS SIZE: " << hmms_size;
//             // std::cerr << "\n PROB_HMMSSSS: " << prob_hmms;
//             prob_move += prob_hmms;
//         }
//         // std::cerr << "\n PROB_MOVE: " << prob_move;
//         prob_move /= COUNT_SPECIES;
//         // std::cerr << "\n PROB_MOVE: " << prob_move;
//         if (prob_move > movePrediction.confidence) {
//             movePrediction.confidence = prob_move;
//             movePrediction.movement = (EMovement)m;
//         }
//         test_movements.pop_back();
//     }
//     return movePrediction;
// }

} /*namespace ducks*/
