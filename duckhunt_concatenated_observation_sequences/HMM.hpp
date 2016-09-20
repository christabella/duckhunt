#ifndef _DUCKS_HMM_HPP_ 
#define _DUCKS_HMM_HPP_

#include "Action.hpp"
#include <vector>

namespace ducks {
    class HMM {
    public:
        HMM();
        HMM(const std::vector<EMovement> &Om);


        double probabilityOfObservationSequence(const std::vector<EMovement> &Om);
        void estimateModel(const std::vector<EMovement> &Om);

    private:
        // void printMatrix(std::vector<std::vector<double>> matrix);
        // void printVector(std::vector<double> vect);

    public:
        std::vector<std::vector<double>> A;
        std::vector<std::vector<double>> B;
        std::vector<double> pi;
        std::vector<int> O_cum;
    };
}

#endif