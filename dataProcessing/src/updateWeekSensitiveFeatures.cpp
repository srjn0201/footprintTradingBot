#include "../dataStructure.h"



void updateWeekChangeSensitiveFeatures(Contract& contract) {
    // Update week change sensitive features
    contract.weeks.back().vwap = 0.0;
    contract.weeks.back().poc = 0.0;    
    contract.weeks.back().vah = 0.0;
    contract.weeks.back().val = 0.0;
    
    
}