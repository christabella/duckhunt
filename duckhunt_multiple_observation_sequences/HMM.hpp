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
        std::vector<std::vector<double>> A_num;
        std::vector<std::vector<double>> A_denom;
        std::vector<std::vector<double>> A_num_temp;
        std::vector<std::vector<double>> A_denom_temp;
        std::vector<std::vector<double>> B;
        std::vector<std::vector<double>> B_num;
        std::vector<std::vector<double>> B_denom;
        std::vector<std::vector<double>> B_num_temp;
        std::vector<std::vector<double>> B_denom_temp;
        std::vector<double> pi;
        std::vector<double> pi_num;
        std::vector<double> pi_denom;
        std::vector<double> pi_num_temp;
        std::vector<double> pi_denom_temp;
    };
}

#endif