#include "../dataStructure.h"



void finalizeProcessingDay(Contract& contract) {
    auto& WEEK = contract.weeks.back();

    Day newDay;
    Bar newBar;

    WEEK.days.push_back(newDay);


}