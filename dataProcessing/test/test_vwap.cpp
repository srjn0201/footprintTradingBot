#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <limits>

// Include the necessary definitions from your project
#include "../dataProcessing/dataStructure.h"

// Forward declare the function we are testing
void updateDayOnNewBar(Contract& contract);

// A helper function to check floating-point equality
void check(double value, double expected) {
    const double epsilon = 1e-5; // A small tolerance for floating point comparisons
    if (std::abs(value - expected) > epsilon) {
        std::cerr << "TEST FAILED: Expected " << expected << ", but got " << value << std::endl;
        assert(false);
    }
}

int main() {
    std::cout << "Running VWAP Test..." << std::endl;

    // 1. Setup: Create a contract and a week/day structure
    Contract contract;
    contract.weeks.push_back(Week());
    contract.weeks.back().days.push_back(Day());

    auto& day = contract.weeks.back().days.back();

    // --- TEST CASE 1: First Bar ---
    std::cout << "Testing first bar..." << std::endl;
    Bar bar1;
    bar1.high = 101.0;
    bar1.low = 99.0;
    bar1.close = 100.0;
    bar1.barTotalVolume = 1000;
    day.bars.push_back(bar1);
    updateDayOnNewBar(contract);

    // Manual Calculation for Bar 1:
    // TP = (101+99+100)/3 = 100
    // VWAP = 100 (since it's the first bar)
    // Variance = 0 (no deviation with one data point)
    check(day.vwap, 100.0);
    check(day.vwapUpperStdDev1, 100.0); // Should equal VWAP when variance is 0
    check(day.vwapLowerStdDev1, 100.0);
    check(day.vwapUpperStdDev2, 100.0);
    check(day.vwapLowerStdDev2, 100.0);
    std::cout << "Test (Bar 1): Passed." << std::endl;


    // --- TEST CASE 2: Second Bar ---
    std::cout << "Testing second bar..." << std::endl;
    Bar bar2;
    bar2.high = 103.0;
    bar2.low = 101.0;
    bar2.close = 102.0;
    bar2.barTotalVolume = 2000;
    day.bars.push_back(bar2);
    updateDayOnNewBar(contract);

    // Manual Calculation for Bar 2:
    // TP1=100, Vol1=1000 -> PV1=100000, SPV1=1000*100^2=10,000,000
    // TP2=(103+101+102)/3=102, Vol2=2000 -> PV2=204000, SPV2=2000*102^2=20,808,000
    // TotalVol = 1000+2000 = 3000
    // CumulPV = 100000+204000 = 304000
    // CumulSPV = 10,000,000+20,808,000 = 30,808,000
    // VWAP = 304000 / 3000 = 101.33333
    // Variance = (30,808,000 / 3000) - (101.33333^2) = 10269.33333 - 10268.44444 = 0.88889
    // StdDev = sqrt(0.88889) = 0.94281
    check(day.vwap, 101.33333);
    check(day.vwapUpperStdDev1, 101.33333 + 0.94281);      // VWAP + 1*StdDev
    check(day.vwapLowerStdDev1, 101.33333 - 0.94281);      // VWAP - 1*StdDev
    check(day.vwapUpperStdDev2, 101.33333 + 2 * 0.94281);  // VWAP + 2*StdDev
    check(day.vwapLowerStdDev2, 101.33333 - 2 * 0.94281);  // VWAP - 2*StdDev
    std::cout << "Test (Bar 2): Passed." << std::endl;


    std::cout << "All VWAP tests passed successfully!" << std::endl;
    return 0;
}