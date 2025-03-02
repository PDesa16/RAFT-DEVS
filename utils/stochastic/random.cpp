#include "random.hpp"


std::random_device rd; // Random device to seed the generator
std::mt19937 gen(rd()); // Mersenne Twister engine for random number generation

// Function to generate a random delay based on an exponential distribution
double RandomNumberGeneratorDEVS::generateExponentialDelay(double lambda) {
    std::exponential_distribution<> dis(lambda);  // Exponential distribution
    return dis(gen);  // Generate and return a random delay based on lambda
}

// Generate a Gaussian delay with mean and standard deviation
double RandomNumberGeneratorDEVS::generateGaussianDelay(double mean, double stddev) {
    std::normal_distribution<> gauss_dist(mean, stddev);
    return gauss_dist(gen);
}

double RandomNumberGeneratorDEVS::generateUniformDelay(double min, double max) {
    // Define the uniform distribution
    std::uniform_real_distribution<> dis(min, max);
    // Generate a random number between min and max
    return dis(gen);
}