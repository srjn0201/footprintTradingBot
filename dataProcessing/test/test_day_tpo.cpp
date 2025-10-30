#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <map>

#include "../dataStructure.h"

// Forward declare the function we are testing
void calculateDayTPO(Contract& contract);

// A helper function to check floating-point equality
void check(double value, double expected, const std::string& testName) {
    const double epsilon = 1e-5;
    if (std::abs(value - expected) > epsilon) {
        std::cerr << "TEST FAILED [" << testName << "]: Expected " << expected << ", but got " << value << std::endl;
        assert(false);
    }
}

void check_int(int64_t value, int64_t expected, const std::string& testName) {
    if (value != expected) {
        std::cerr << "TEST FAILED [" << testName << "]: Expected " << expected << ", but got " << value << std::endl;
        assert(false);
    }
}

// Helper to set up a contract with one empty day
void setupContract(Contract& contract) {
    contract.weeks.push_back(Week());
    contract.weeks.back().days.push_back(Day());
}

int main() {
    std::cout << "Running TPO/Volume Profile Test..." << std::endl;

    // --- Test 1: Empty Day ---
    {
        Contract contract;
        setupContract(contract);
        calculateDayTPO(contract);

        check(contract.weeks.back().days.back().poc, 0.0, "Empty Day POC");
        check(contract.weeks.back().days.back().vah, 0.0, "Empty Day VAH");
        check(contract.weeks.back().days.back().val, 0.0, "Empty Day VAL");
        check_int(contract.weeks.back().days.back().totalVolume, 0, "Empty Day Volume");
        std::cout << "Test (Empty Day): Passed." << std::endl;
    }

    // --- Test 2: Simple Symmetric Profile ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        day.bars.resize(1); // One bar for simplicity
        day.bars[0].footprint.priceLevels = {
            { 98.0, {50, 50}},   // 100
            { 99.0, {100, 100}}, // 200
            {100.0, {250, 250}}, // 500 (POC)
            {101.0, {100, 100}}, // 200 (HVN)
            {102.0, {50, 50}}    // 100
        };
        // Total Volume = 1100. Target VA Volume = 1100 * 0.7 = 770.
        // 1. Start at POC (100.0): vol = 500.
        // 2. Add larger of neighbors (99/101 are equal, code picks lower): vol = 500+200=700. VAL=99.
        // 3. Add larger of neighbors (101 vs 98): vol = 700+200=900. VAH=101. Loop ends.
        calculateDayTPO(contract);
        check(day.poc, 100.0, "Symmetric POC");
        check(day.lastHighVolumeNode, 99.0, "Symmetric HVN");
        check(day.vah, 101.0, "Symmetric VAH");
        check(day.val, 99.0, "Symmetric VAL");
        check_int(day.totalVolume, 1100, "Symmetric Volume");
        std::cout << "Test (Symmetric Profile): Passed." << std::endl;
    }

    // --- Test 3: Asymmetric Profile ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        day.bars.resize(1);
        day.bars[0].footprint.priceLevels = {
            {100.0, {50, 50}},   // 100
            {101.0, {200, 200}}, // 400 (HVN)
            {102.0, {500, 500}}, // 1000 (POC)
            {103.0, {150, 150}}, // 300
            {104.0, {25, 25}}    // 50
        };
        // Total Volume = 1850. Target VA Volume = 1850 * 0.7 = 1295.
        // 1. Start at POC (102.0): vol = 1000.
        // 2. Add larger neighbor (101 vs 103): vol = 1000+400=1400. VAL=101. Loop ends.
        calculateDayTPO(contract);
        check(day.poc, 102.0, "Asymmetric POC");
        check(day.lastHighVolumeNode, 101.0, "Asymmetric HVN");
        check(day.vah, 102.0, "Asymmetric VAH");
        check(day.val, 101.0, "Asymmetric VAL");
        check_int(day.totalVolume, 1850, "Asymmetric Volume");
        std::cout << "Test (Asymmetric Profile): Passed." << std::endl;
    }

    // --- Test 4: Profile with a Gap ---
    {
        Contract contract;
        setupContract(contract);
        auto& day = contract.weeks.back().days.back();
        day.bars.resize(1);
        day.bars[0].footprint.priceLevels = {
            {100.0, {250, 250}}, // 500
            {101.0, {400, 400}}, // 800 (POC)
            // Gap at 102.0
            {103.0, {300, 300}}  // 600 (HVN)
        };
        // Total Volume = 1900. Target VA Volume = 1900 * 0.7 = 1330.
        // 1. Start at POC (101.0): vol = 800.
        // 2. Add larger neighbor (103 vs 100): vol = 800+600=1400. VAH=103. Loop ends.
        calculateDayTPO(contract);
        check(day.poc, 101.0, "Gap POC");
        check(day.lastHighVolumeNode, 103.0, "Gap HVN");
        check(day.vah, 103.0, "Gap VAH");
        check(day.val, 101.0, "Gap VAL");
        check_int(day.totalVolume, 1900, "Gap Volume");
        std::cout << "Test (Gap Profile): Passed." << std::endl;
    }

    std::cout << "All TPO tests passed successfully!" << std::endl;
    return 0;
}