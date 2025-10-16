#include "../dataStructure.h"



void checkForSignal(Contract& contract){
    auto& BAR = contract.weeks.back().days.back().bars.back();
    // this is the demo logic for signal checking
    BAR.signal = 1;
    BAR.signalID = 1;
    BAR.signalStatus = true;

}