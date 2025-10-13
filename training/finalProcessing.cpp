#include "dataStructure.h"
#include <vector>
#include <numeric>
#include <random>
#include <chrono>
#include <ctime>
#include <iostream>
#include "dataStructure.h"
#include "database/database.h"
#include "src/updatefeatures.h"


// this function will take inputs contract and the weekVector and database path
void finalProcessing(int bar_range, Contract& contract , std::vector<weekVector>& weeksVector, const std::string& database_path, const std::string& table_name) {
    // start itteration on the weeksVector
    for (const auto& processing_week : weeksVector) {
        // the start new loop for days in that processing_week
        for (const auto& processing_day : processing_week.days) {
            // fetch the data for that iterated processing_day
            // this is due to the fact that our database has different datetime format
            Date processing_date = {
                processing_day.date.y,
                processing_day.date.m,
                processing_day.date.d
            };
            std::vector<TickData> processing_day_data = fetchData(database_path, table_name, processing_date);
            std::cout << "processing data for :" << processing_date.y << "-" << processing_date.m << "-" << processing_date.d <<"\n";               

                // start new main data processing loop iterating through each row of the fetch data
                for (const auto& row : processing_day_data) {
                    double currentPrice = row.Price;
                    int currentAskVolume = row.AskVolume;
                    int currentBidVolume = row.BidVolume;
                    std::string currentTime = row.DateTime;
                    double lastHigh = contract.weeks.back().days.back().bars.back().high;
                    double lastLow = contract.weeks.back().days.back().dayLow;
                
                    // check if the price is in the range of the current bar, this due to the fact that we are processing chart in ranged bar
                    if (lastHigh - currentPrice <= bar_range || currentPrice - lastLow <= bar_range)  {          //if price is in the range so only updation of the bar
                        if (currentPrice != contract.weeks.back().days.back().bars.back().close) {           //if price changed
                            
                            // update footprint bar
                            updateFootprint(contract, currentPrice, currentAskVolume, currentBidVolume);

                            checkForSignal(contract);      //check for signal
                            // if signal 
                                //update all
                                // append to signal data structure

                            // update tick change sensitive features
                            updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                            // update price change sensitive feartures
                            updatePriceSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);
                        }    

                        else  {         //else price not changed
                            // update footprint bar
                            updateFootprint(contract, currentPrice, currentAskVolume, currentBidVolume);

                            checkForSignal(contract);      //check for signal
                                // if signal
                                    //update all
                                    // append to signal data structure

                            // update tick change sensitive features
                            updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                        }
                    }
                    else {           //else price not in the range, so we need to create new bar
                        // finalize the last bar and update bar change sensitive features
                        // and add new bar in the bars vector of the currect processing day's data structure
                        finalizeLastBar(contract, currentTime, currentPrice, currentAskVolume, currentBidVolume);
                        updateBarChangeSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                        // update footprint bar
                        updateFootprint(contract, currentPrice, currentAskVolume, currentBidVolume);

                        // update tick change sensitive features
                        updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                        // update price change sensitive feartures
                        updatePriceSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                    }
                }
            // finalize the processing_day and update day change sensitive features
            finalizeProcessingDay(contract);
            updateDayChangeSensitiveFeatures(contract);
        }
        // finalize the processing_week and update week change sensitive features
        finalizeProcessingWeek(contract);
        updateWeekChangeSensitiveFeatures(contract);

    }
    // finalize the contract
    finalizeContract(contract);
}
