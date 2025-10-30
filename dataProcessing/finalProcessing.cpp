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

extern void initializeNewDay(Contract& contract, double firstPrice);




// this function will take inputs contract and the weekVector and database path
void finalProcessing(double bar_range, double imbalanceThreshhold, Contract& contract , std::vector<weekVector>& weeksVector, const std::string& database_path, const std::string& table_name) {
    // start itteration on the weeksVector
    for (const auto& processing_week : weeksVector) {
        // the start new loop for days in that processing_week
        std::cout <<"starting for the week : " << processing_week.weekNumber <<std::endl;
        for (const auto& processing_day : processing_week.days) {
            // fetch the data for that iterated processing_day
            // this is due to the fact that our database has different datetime format
            Date processing_date = {
                processing_day.date.y,
                processing_day.date.m,
                processing_day.date.d
            };
            std::vector<TickData> processing_day_data = fetchData(database_path, table_name, processing_date);
            std::cout << "starting processing data for :" << processing_date.y << "-" << processing_date.m << "-" << processing_date.d << "  datasize:" << processing_day_data.size() << "\n";               

            // initialising new day with values
            if (processing_day_data.empty()) {
                std::cout << "No data found for the day" << std::endl;
                continue;
            }
            else{
                initializeNewDay(contract, processing_day_data.front().Price);
                std::cout << "new day initializes, and starting populating the data to the new day struct" << std::endl;
            }

                // start new main data processing loop iterating through each row of the fetch data
            for (const auto& row : processing_day_data) {
                if (processing_day_data.empty()) {
                    std::cout << "No data found for date: " << processing_date.y << "-" << processing_date.m << "-" << processing_date.d << std::endl;
                    continue; // Skip to the next day if no data is found
                }
                // std::cout << "st"
                double currentPrice = row.Price;
                int currentAskVolume = row.AskVolume;
                int currentBidVolume = row.BidVolume;
                std::string currentTime = row.DateTime;

                double lastHigh = contract.weeks.back().days.back().bars.back().high;
                double lastLow = contract.weeks.back().days.back().bars.back().low;
            
                // check if the price is in the range of the current bar, this due to the fact that we are processing chart in ranged bar
                if (lastHigh - currentPrice <= bar_range && currentPrice - lastLow <= bar_range )  {          //if price is in the range so only updation of the bar
                    if (currentPrice != contract.weeks.back().days.back().bars.back().close) {           //if price changed
                        // updating bar's ohlc
                        contract.weeks.back().days.back().bars.back().close = currentPrice;
                        contract.weeks.back().days.back().bars.back().high = std::max(lastHigh, currentPrice);
                        contract.weeks.back().days.back().bars.back().low = std::min(lastLow, currentPrice);
                        contract.weeks.back().days.back().bars.back().endTime = currentTime;

                        // update footprint bar
                        auto imbalance_change = updateFootprint(contract, imbalanceThreshhold, currentPrice, currentAskVolume, currentBidVolume);

                        checkForSignal(contract);      //check for signal
                        // if signal 
                            //update all
                            // append to signal data structure

                        // update tick change sensitive features
                        updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume, imbalance_change);

                        // update price change sensitive feartures
                        updatePriceSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);
                    }    

                    else  {         //else price not changed
                        // update footprint bar
                        auto imbalance_change = updateFootprint(contract, imbalanceThreshhold, currentPrice, currentAskVolume, currentBidVolume);

                        checkForSignal(contract);      //check for signal
                            // if signal
                                //update all
                                // append to signal data structure

                        // update tick change sensitive features
                        updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume, imbalance_change);

                    }
                }
                else {           //else price not in the range, so we need to create new bar
                    // finalize the last bar and update bar change sensitive features
                    // and add new bar in the bars vector of the currect processing day's data structure
                    updateBarChangeSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);
                    initializeNewBar(contract, currentTime, currentPrice, currentAskVolume, currentBidVolume);
                    
                    // update footprint bar
                    auto imbalance_change = updateFootprint(contract, imbalanceThreshhold, currentPrice, currentAskVolume, currentBidVolume);

                    // update tick change sensitive features
                    updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume, imbalance_change);

                    // update price change sensitive feartures
                    updatePriceSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                }
            }
            // finalize the processing_day and update day change sensitive features
            updateDayChangeSensitiveFeatures(contract);
            // finalizeProcessingDay(contract);
            std::cout <<"day processing finished" << std::endl;
        }
        // finalize the processing_week and update week change sensitive features
        std::cout <<"week processing finished" << std::endl;
        updateWeekChangeSensitiveFeatures(contract);
        finalizeProcessingWeek(contract);

    }
    // finalize the contract
    finalizeContract(contract);
}
