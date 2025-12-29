#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

// --- The REAL Data Structure ---
// REMOVE all mock structs and include the actual header file.
// This ensures the test and the production code use the exact same object layout.
#include "../dataStructure.h"

// Forward declare the function we are testing
void calculateDeltaZscore(Contract& contract);

// A helper function to check floating-point equality
void check(double value, double expected, const std::string& testName) {
    const double epsilon = 1e-5;
    if (std::abs(value - expected) > epsilon) {
        std::cerr << "TEST FAILED [" << testName << "]: Expected " << expected << ", but got " << value << std::endl;
        assert(false);
    }
}

// Helper to set up a contract with one empty day
void setupContract(Contract& contract) {
    contract.weeks.push_back(Week());
    contract.weeks.back().days.push_back(Day());
}

// Helper to create a bar and set its delta explicitly
Bar createBarWithDelta(int deltaValue) {
    Bar bar; // This now creates the REAL, complex Bar object
    bar.delta = deltaValue;
    return bar;
}

int main() {
    std::cout << "Running Delta Z-score Test..." << std::endl;

    // --- Test 1: n = 1 ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        day.bars.push_back(createBarWithDelta(10));
        calculateDeltaZscore(contract);

        check(day.deltaSum, 10.0, "n=1 Sum");
        check(day.deltaSumOfSquares, 100.0, "n=1 SumOfSquares");
        check(day.deltaZscore20bars, 10.0, "n=1 Z-score"); // Per code logic
        std::cout << "Test (n=1): Passed." << std::endl;
    }

    // --- Test 2: n = 2 (Seeding) ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        day.bars.push_back(createBarWithDelta(10));
        calculateDeltaZscore(contract); // n=1 call
        day.bars.push_back(createBarWithDelta(12));
        calculateDeltaZscore(contract); // n=2 call

        // Manual Calc: mean=(10+12)/2=11, var=(100+144)/2 - 11^2 = 1, stdDev=1
        // Z = (12 - 11) / 1 = 1
        check(day.deltaZscore20bars, 1.0, "n=2 Z-score");
        std::cout << "Test (n=2 Seeding): Passed." << std::endl;
    }

    // --- Test 3: n = 20 (End of Seeding) ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        for (int i = 1; i <= 20; ++i) {
            day.bars.push_back(createBarWithDelta(i));
            calculateDeltaZscore(contract);
        }
        // Manual Calc: deltas 1..20. mean=10.5, var=33.25, stdDev=5.76628
        // Z = (20 - 10.5) / 5.76628 = 1.6475
        check(day.deltaZscore20bars, 1.64751, "n=20 Z-score");
        check(day.deltaSum, 210.0, "n=20 Sum");
        check(day.deltaSumOfSquares, 2870.0, "n=20 SumOfSquares");
        std::cout << "Test (n=20 Seeding): Passed." << std::endl;
    }

    // --- Test 4: n = 21 (First Rolling Update) ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        for (int i = 1; i <= 21; ++i) {
            day.bars.push_back(createBarWithDelta(i));
            calculateDeltaZscore(contract);
        }
        // Manual Calc: window is 2..21. old=1, new=21.
        // newSum = 210-1+21=230, newSumSq=2870-1+441=3310
        // mean=230/20=11.5, var=(3310/20)-11.5^2=33.25, stdDev=5.76628
        // Z = (21 - 11.5) / 5.76628 = 1.6475
        check(day.deltaZscore20bars, 1.64751, "n=21 Z-score");
        check(day.deltaSum, 230.0, "n=21 Sum");
        check(day.deltaSumOfSquares, 3310.0, "n=21 SumOfSquares");
        std::cout << "Test (n=21 Rolling): Passed." << std::endl;
    }

    // --- Test 5: Zero Standard Deviation ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        for (int i = 1; i <= 20; ++i) {
            day.bars.push_back(createBarWithDelta(5)); // All deltas are the same
            calculateDeltaZscore(contract);
        }
        // StdDev is 0, so Z-score should be 0
        check(day.deltaZscore20bars, 0.0, "Zero StdDev Z-score");
        std::cout << "Test (Zero StdDev): Passed." << std::endl;
    }

    std::cout << "All Delta Z-score tests passed successfully!" << std::endl;
    return 0;
}