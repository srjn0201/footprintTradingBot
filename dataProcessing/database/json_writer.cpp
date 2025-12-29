#include "json_writer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath> // For std::isnan

// Helper function to convert double to JSON-compatible string, handling NaN
std::string doubleToJson(double value) {
    if (std::isnan(value)) {
        return "null";
    }
    return std::to_string(value);
}

// Forward declarations for serialization functions
std::string priceLevelToJson(double price, const PriceLevel& pl);
std::string footprintToJson(const Footprint& fp);
std::string barToJson(const Bar& bar);
std::string dayToJson(const Day& day);
std::string weekToJson(const Week& week);
std::string contractToJson(const Contract& contract);

std::string priceLevelToJson(double price, const PriceLevel& pl) {
    std::stringstream ss;
    ss << "{";
    ss << "\"price\": " << doubleToJson(price) << ",";
    ss << "\"bidVolume\": " << pl.bidVolume << ",";
    ss << "\"askVolume\": " << pl.askVolume << ",";
    ss << "\"isBuyImbalance\": " << (pl.isBuyImbalance ? "true" : "false") << ",";
    ss << "\"isSellImbalance\": " << (pl.isSellImbalance ? "true" : "false") << ",";
    ss << "\"volumeAtPrice\": " << pl.volumeAtPrice << ",";
    ss << "\"deltaAtPrice\": " << pl.deltaAtPrice;
    ss << "}";
    return ss.str();
}

std::string footprintToJson(const Footprint& fp) {
    std::stringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& [price, level] : fp.priceLevels) {
        if (!first) {
            ss << ",";
        }
        ss << priceLevelToJson(price, level);
        first = false;
    }
    ss << "]";
    return ss.str();
}

std::string barToJson(const Bar& bar) {
    std::stringstream ss;
    ss << "{";
    ss << "\"startTime\": \"" << bar.startTime << "\",";
    ss << "\"endTime\": \"" << bar.endTime << "\",";
    ss << "\"open\": " << doubleToJson(bar.open) << ",";
    ss << "\"high\": " << doubleToJson(bar.high) << ",";
    ss << "\"low\": " << doubleToJson(bar.low) << ",";
    ss << "\"close\": " << doubleToJson(bar.close) << ",";
    ss << "\"barTotalVolume\": " << bar.barTotalVolume << ",";
    ss << "\"footprint\": " << footprintToJson(bar.footprint) << ",";
    ss << "\"buyImbalanceCount\": " << bar.buyImbalanceCount << ",";
    ss << "\"sellImbalanceCount\": " << bar.sellImbalanceCount << ",";
    ss << "\"delta\": " << bar.delta << ",";
    ss << "\"barDeltaChange\": " << bar.barDeltaChange << ",";
    ss << "\"barHighDelta\": " << bar.barHighDelta << ",";
    ss << "\"barLowDelta\": " << bar.barLowDelta << ",";
    ss << "\"barPOCPrice\": " << doubleToJson(bar.barPOCPrice) << ",";
    ss << "\"barPOCVol\": " << bar.barPOCVol << ",";    
    ss << "\"signal\": " << bar.signal << ",";
    ss << "\"signalID\": " << bar.signalID << ",";
    ss << "\"signalStatus\": " << (bar.signalStatus ? "true" : "false") << ",";
    ss << "\"priceCurrentDayVwapDiff\": " << doubleToJson(bar.priceCurrentDayVwapDiff) << ",";
    ss << "\"priceCurrentDayVwapUpperStdDevDiff\": " << doubleToJson(bar.priceCurrentDayVwapUpperStdDev1Diff) << ",";
    ss << "\"priceCurrentDayVwapLowerStdDevDiff\": " << doubleToJson(bar.priceCurrentDayVwapLowerStdDev1Diff) << ",";
    ss << "\"priceCurrentDayVwapUpperStdDevDiff\": " << doubleToJson(bar.priceCurrentDayVwapUpperStdDev2Diff) << ",";
    ss << "\"priceCurrentDayVwapLowerStdDevDiff\": " << doubleToJson(bar.priceCurrentDayVwapLowerStdDev2Diff) << ",";
    ss << "\"pricePreviousDayVwapDiff\": " << doubleToJson(bar.pricePreviousDayVwapDiff) << ",";
    ss << "\"priceWeeklyVwapDiff\": " << doubleToJson(bar.priceWeeklyVwapDiff) << ",";
    ss << "\"priceBBandUpperDiff\": " << doubleToJson(bar.priceBBandUpperDiff) << ",";
    ss << "\"priceBBandLowerDiff\": " << doubleToJson(bar.priceBBandLowerDiff) << ",";
    ss << "\"PriceBBandMiddleDiff\": " << doubleToJson(bar.PriceBBandMiddleDiff) << ",";
    ss << "\"isPriceInCurrentDayVA\": " << (bar.isPriceInCurrentDayVA ? "true" : "false") << ",";
    ss << "\"isPriceInPrevDayVA\": " << (bar.isPriceInPrevDayVA ? "true" : "false") << ",";
    ss << "\"priceCurrDayVAHDiff\": " << doubleToJson(bar.priceCurrDayVAHDiff) << ",";
    ss << "\"priceCurrDayVALDiff\": " << doubleToJson(bar.priceCurrDayVALDiff) << ",";
    ss << "\"pricePrevDayPOCDiff\": " << doubleToJson(bar.pricePrevDayPOCDiff) << ",";
    ss << "\"pricePrevDayVAHDiff\": " << doubleToJson(bar.pricePrevDayVAHDiff) << ",";
    ss << "\"pricePrevDayVALDiff\": " << doubleToJson(bar.pricePrevDayVALDiff) << ",";
    ss << "\"priceIBHighDiff\": " << doubleToJson(bar.priceIBHighDiff) << ",";
    ss << "\"priceIBLowDiff\": " << doubleToJson(bar.priceIBLowDiff) << ",";
    ss << "\"pricePrevDayHighDiff\": " << doubleToJson(bar.pricePrevDayHighDiff) << ",";
    ss << "\"pricePrevDayLowDiff\": " << doubleToJson(bar.pricePrevDayLowDiff) << ",";
    ss << "\"pricePrevDayCloseDiff\": " << doubleToJson(bar.pricePrevDayCloseDiff) << ",";
    ss << "\"priceCurrentWeekHighDiff\": " << doubleToJson(bar.priceCurrentWeekHighDiff) << ",";
    ss << "\"priceCurrentWeekLowDiff\": " << doubleToJson(bar.priceCurrentWeekLowDiff) << ",";
    ss << "\"pricePrevWeekHighDiff\": " << doubleToJson(bar.pricePrevWeekHighDiff) << ",";
    ss << "\"pricePrevWeekLowDiff\": " << doubleToJson(bar.pricePrevWeekLowDiff) << ",";
    ss << "\"priceLastSwingHighDiff\": " << doubleToJson(bar.priceLastSwingHighDiff) << ",";
    ss << "\"priceLastSwingLowDiff\": " << doubleToJson(bar.priceLastSwingLowDiff) << ",";
    ss << "\"priceLastHVNDiff\": " << doubleToJson(bar.priceLastHVNDiff);
    ss << "}";
    return ss.str();
}

std::string dayToJson(const Day& day) {
    std::stringstream ss;
    ss << "{";
    ss << "\"dayOfTheWeek\": \"" << day.dayOfTheWeek << "\",";
    ss << "\"bars\": [";
    bool first = true;
    for (const auto& bar : day.bars) {
        if (!first) {
            ss << ",";
        }
        ss << barToJson(bar);
        first = false;
    }
    ss << "]";
    ss << ",";
    ss << "\"deltaZscore20bars\": " << doubleToJson(day.deltaZscore20bars) << ",";
    ss << "\"cumDelta5barSlope\": " << doubleToJson(day.cumDelta5barSlope) << ",";
    ss << "\"priceCumDeltaDivergence5bar\": " << doubleToJson(day.priceCumDeltaDivergence5bar) << ",";
    ss << "\"priceCumDeltaDivergence10bar\": " << doubleToJson(day.priceCumDeltaDivergence10bar) << ",";
    ss << "\"interactionReversal20bar\": " << doubleToJson(day.interactionReversal20bar) << ",";
    ss << "\"vwap\": " << doubleToJson(day.vwap) << ",";
    ss << "\"vwapUpperStdDev1\": " << doubleToJson(day.vwapUpperStdDev1) << ",";
    ss << "\"vwapUpperStdDev2\": " << doubleToJson(day.vwapUpperStdDev2) << ",";
    ss << "\"vwapLowerStdDev1\": " << doubleToJson(day.vwapLowerStdDev1) << ",";
    ss << "\"vwapLowerStdDev2\": " << doubleToJson(day.vwapLowerStdDev2) << ",";
    ss << "\"vwapBandWidth\": " << doubleToJson(day.vwapBandWidth) << ",";
    ss << "\"bbMiddle\": " << doubleToJson(day.bbMiddle) << ",";
    ss << "\"bbUpper\": " << doubleToJson(day.bbUpper) << ",";
    ss << "\"bbLower\": " << doubleToJson(day.bbLower) << ",";
    ss << "\"BBandWidth\": " << doubleToJson(day.BBandWidth) << ",";
    ss << "\"rsi\": " << doubleToJson(day.rsi) << ",";
    ss << "\"poc\": " << doubleToJson(day.poc) << ",";
    ss << "\"vah\": " << doubleToJson(day.vah) << ",";
    ss << "\"val\": " << doubleToJson(day.val) << ",";
    ss << "\"ibHigh\": " << doubleToJson(day.ibHigh) << ",";
    ss << "\"ibLow\": " << doubleToJson(day.ibLow) << ",";
    ss << "\"dayHigh\": " << doubleToJson(day.dayHigh) << ",";
    ss << "\"dayLow\": " << doubleToJson(day.dayLow) << ",";
    ss << "\"dayClose\": " << doubleToJson(day.dayClose) << ",";
    ss << "\"totalVolume\": " << day.totalVolume << ",";
    ss << "\"cumulativeDelta\": " << day.cumulativeDelta << ",";
    ss << "\"lastSwingHigh\": " << doubleToJson(day.lastSwingHigh) << ",";
    ss << "\"lastSwingLow\": " << doubleToJson(day.lastSwingLow) << ",";
    ss << "\"lastHighVolumeNode\": " << doubleToJson(day.lastHighVolumeNode) << ",";
    ss << "\"prevAvgGain\": " << doubleToJson(day.prevAvgGain) << ",";
    ss << "\"prevAvgLoss\": " << doubleToJson(day.prevAvgLoss) << ",";
    ss << "\"variance\": " << doubleToJson(day.variance) << ",";
    ss << "\"cumulativePV\": " << doubleToJson(day.cumulativePV) << ",";
    ss << "\"cumulativeSquarePV\": " << doubleToJson(day.cumulativeSquarePV);
    ss << "}";
    return ss.str();
}

std::string weekToJson(const Week& week) {
    std::stringstream ss;
    ss << "{";
    ss << "\"weekOfTheContract\": \"" << week.weekOfTheContract << "\",";
    ss << "\"days\": [";
    bool first = true;
    for (const auto& day : week.days) {
        if (!first) {
            ss << ",";
        }
        ss << dayToJson(day);
        first = false;
    }
    ss << "]";
    ss << ",";
    ss << "\"totalVolume\": " << doubleToJson(week.totalVolume) << ",";
    ss << "\"vwap\": " << doubleToJson(week.vwap) << ",";
    ss << "\"poc\": " << doubleToJson(week.poc) << ",";
    ss << "\"vah\": " << doubleToJson(week.vah) << ",";
    ss << "\"val\": " << doubleToJson(week.val) << ",";
    ss << "\"weekHigh\": " << doubleToJson(week.weekHigh) << ",";
    ss << "\"weekLow\": " << doubleToJson(week.weekLow);
    ss << "}";
    return ss.str();
}

std::string contractToJson(const Contract& contract) {
    std::stringstream ss;
    ss << "{";
    ss << "\"contractName\": \"" << contract.contractName << "\",";
    ss << "\"weeks\": [";
    bool first = true;
    for (const auto& week : contract.weeks) {
        if (!first) {
            ss << ",";
        }
        ss << weekToJson(week);
        first = false;
    }
    ss << "]";
    ss << "}";
    return ss.str();
}

void writeContractToJson(const Contract& contract, const std::string& output_dir) {
    std::string json_string = contractToJson(contract);
    std::ofstream outfile(output_dir + "/contract.json");
    outfile << json_string;
    outfile.close();
}
