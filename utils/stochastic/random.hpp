#ifndef RANDOM_DEVS_HPP
#define RANDOM_DEVS_HPP

#include <random>
#include <cmath>

// Holds definitions for Random Number Generations
class RandomNumberGeneratorDEVS {
    public:
        static double generateExponentialDelay(double lambda);
        static double generateGaussianDelay(double mean, double stddev);
        static double generateUniformDelay(double min, double max);
};

#endif