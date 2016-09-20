#include "HMM.hpp"
#include <cmath>
#include <vector>
#include <random>
#include <iomanip>

#define N 5
#define M COUNT_MOVE
#define MAX_ITERS 250

namespace ducks {
    // void HMM::printMatrix(std::vector<std::vector<double>> matrix) {
    //     std::vector<std::vector<double>>::iterator it;
    //     std::vector<double>::iterator inner;
    //     for (it=matrix.begin(); it != matrix.end(); it++) {
    //         for (inner = it->begin(); inner != it->end(); inner++) {
    //             std::cerr << std::setw(10) << *inner;
    //             if(inner+1 != it->end()) {
    //                 std::cerr << "\t";
    //             }
    //         }
    //         std::cerr << std::endl;
    //     }
    // }
    
    // void HMM::printVector(std::vector<double> vect) {
    //     std::vector<double>::iterator it;
    //     for (it=vect.begin(); it != vect.end(); it++) {
    //         std::cerr << *it;
    //         std::cerr << " ";
    //     }
    //     std::cerr << std::endl;
    // }
    HMM::HMM() : 
        A(N, std::vector<double>(N, 0)),
        B(N, std::vector<double>(M, 0)),
        pi(N, 0)
    {
        // duplicate of HMM::HMM(Om)
        std::cerr << "INITIALIZING HMM|\n";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);
        std::uniform_real_distribution<> dis_pi(0.15, 0.25);

        // Initialize pi
        double sum_row = 0;

        for (int i = 0; i < N; ++i) {
            // pi[i] = 1/N;
            pi[i] = dis_pi(gen);
            sum_row += pi[i];
        }
        // Normalize pi
        for (int i = 0; i < N; ++i) 
            pi[i] /= sum_row;
        

        // Initialize A
        std::vector<double> sum_rows(N);

        for (int i = 0; i < N; ++i) {
            sum_rows[i] = 0;
            for (int j = 0; j < N; ++j) {
                if (i != j) {           
                    A[i][j] = 0.125;
                    // A[i][j] = dis(gen);
                    // sum_rows[i] += A[i][j];
                } else {
                    A[i][j] = 0.5;
                    // A[i][j] = 4;
                    // sum_rows[i] += 4;
                }
            }
        }
        // for (int i = 0; i < N; ++i) {
        //     for (int j = 0; j < N; ++j) {
        //         A[i][j] /= sum_rows[i];
        //     }
        // }

        // Initialize B
        for (int i = 0; i < N; ++i) {
            sum_rows[i] = 0;
            for (int j = 0; j < M; ++j) {
                B[i][j] = dis(gen);
                sum_rows[i] += B[i][j];
            }
        }
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                B[i][j] /= sum_rows[i];
            }
        }
        std::cerr << "IN HMM|\n";
    }    
    double HMM::probabilityOfObservationSequence(const std::vector<EMovement> &Om) {
        int T = Om.size();
        // std::cerr << "POOS|\n";

        // Convert EMovement to int
        std::vector<int> O(T);
        for (int i = 0; i < T; ++i) {
            O[i] = Om[i];
        }
    
        std::vector<double> prev_alpha(N);
        std::vector<double> alpha(N);
        // Compute α_0
        for (int i = 0; i < N; i++) {
            prev_alpha[i] = pi[i] * B[i][O[0]]; // α_0
        }

        // Compute α_t
        for (int t = 1; t < T; t++) {
            for (int i = 0; i < N; i++) {
                alpha[i] = 0;
                for (int j = 0; j < N; j++) {
                    alpha[i] += prev_alpha[j] * A[j][i];
                }
                alpha[i] *= B[i][O[t]];
            }
            prev_alpha = alpha;
        }

        // Sum over all states at α_T
        double probability = 0;
        for (int i = 0; i < N; ++i) {
            probability += alpha[i];
        }

        return probability;
    }

    void HMM::estimateModel(const std::vector<EMovement> &Om) {
        if (O_cum.size() > 700)
            return;

        int T = Om.size();

        // Convert EMovement to int
        for (int i = 0; i < T; ++i) {
            O_cum.push_back(Om[i]);
        }

        T = O_cum.size();
        std::cerr << "O_CUM_SIZE: " << T << std::endl;
        int iters = 0;
        int maxIters = MAX_ITERS;
        double logProb = -999999999999990;
        double oldLogProb = -999999999999999;

        std::vector<double> c(T);
        std::vector<std::vector<double>> alpha(T, std::vector<double>(N));
        std::vector<std::vector<double>> beta(T, std::vector<double>(N));
        std::vector<std::vector<double>> gamma(T, std::vector<double>(N));
        std::vector<std::vector<std::vector<double>>> di_gamma(T, std::vector<std::vector<double>>(N, std::vector<double>(N)));


        while (iters < maxIters && logProb > oldLogProb + 0.0000001) {
            /********** α-PASS **********/
            // Compute α_0
            c[0] = 0;
            for (int i = 0; i < N; i++) {
                alpha[0][i] = pi[i] * B[i][O_cum[0]]; // α_0
                c[0] += alpha[0][i];
            }

            // Scale α_0
            c[0] = 1/c[0];
            for (int i = 0; i < N; i++) {
                alpha[0][i] *= c[0];
            }


            // Compute α_t
            for (int t = 1; t < T; t++) {
                c[t] = 0;
                for (int i = 0; i < N; i++) {
                    alpha[t][i] = 0;
                    for (int j = 0; j < N; j++) {
                        alpha[t][i] += alpha[t-1][j] * A[j][i];
                    }
                    alpha[t][i] *= B[i][O_cum[t]];
                    c[t] += alpha[t][i];
                }
                // Scale α_t
                c[t] = 1/c[t];
                for (int i = 0; i < N; i++) {
                    alpha[t][i] *= c[t];
                }
            }

            /********** β-PASS **********/
            // Let βT−1(i) = 1, scaled by cT−1
            for (int i = 0; i < N; i++) {
                beta[T-1][i] = c[T-1];
            }

            // Computer β_t
            for (int t = T-2; t >= 0; t--) {
                for (int i = 0; i < N; i++) {
                    beta[t][i] = 0;
                    for (int j = 0; j < N; j++) {
                        beta[t][i] += A[i][j] * B[j][O_cum[t+1]] * beta[t+1][j];
                    }
                    // Scale β_t
                    beta[t][i] *= c[t];
                }
            }

            /********** GAMMA AND DI-GAMMA **********/
            double denom;
            // Compute γ_t(i, j) and γ_t(i)
            for (int t = 0; t < T-1; t++) {
                denom = 0;
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        denom += alpha[t][i] * A[i][j] * B[j][O_cum[t+1]] * beta[t+1][j];
                    }
                }
                for (int i = 0; i < N; i++) {
                    gamma[t][i] = 0;
                    for (int j = 0; j < N; j++) {
                        di_gamma[t][i][j] = (alpha[t][i] * A[i][j] * B[j][O_cum[t+1]] * beta[t+1][j])/denom;
                        gamma[t][i] += di_gamma[t][i][j];
                    }
                }
            }

            // Special case for γ_T-1(i)
            denom = 0;
            for (int i = 0; i < N; ++i) {
                denom += alpha[T-1][i];
            }
            for (int i = 0; i < N; ++i) {
                gamma[T-1][i] = alpha[T-1][i] / denom;
            }

            /********** Re-estimate A, B and pi⇡ **********/

            // Re-estimate pi
            for (int i = 0; i < N; ++i){
                pi[i] = gamma[0][i];
            }

            double numer;
            // Re-estimate A
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    numer = 0;
                    denom = 0;
                    for (int t = 0; t < T-1; t++) {
                        numer += di_gamma[t][i][j];
                        denom += gamma[t][i];
                    }
                    A[i][j] = numer / denom;
                }
            }

            // Re-estimate B
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < M; ++j) {
                    numer = 0;
                    denom = 0;
                    for (int t = 0; t < T; t++) {
                        if (O_cum[t] == j) {
                            numer += gamma[t][i];
                        }
                        denom += gamma[t][i];
                    }
                    B[i][j] = numer / denom;
                }
            }

            // Compute log[P(O_cum|lambda)]
            oldLogProb = logProb;
            logProb = 0;
            for (int i = 0; i < T; ++i) {
                logProb += log(c[i]);
            }
            logProb = -logProb;

            // To iterate or not to iterate, that is the question...
            iters += 1;
        } // end while

        // std::cerr << "----------A----------\n";
        // printMatrix(A);
        // std::cerr << "----------B----------\n";
        // printMatrix(B);
        // std::cerr << "----------pi----------\n";
        // printVector(pi);
    } // end estimateModel
} // end namespace ducks
