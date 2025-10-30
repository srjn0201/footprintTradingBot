#include "json_writer.h"
#include <iostream>
#include <fstream>
#include <sstream>

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
    ss << "\"price\": " << price << ",";
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
    ss << "\"open\": " << bar.open << ",";
    ss << "\"high\": " << bar.high << ",";
    ss << "\"low\": " << bar.low << ",";
    ss << "\"close\": " << bar.close << ",";
    ss << "\"barTotalVolume\": " << bar.barTotalVolume << ",";
    ss << "\"footprint\": " << footprintToJson(bar.footprint) << ",";
    ss << "\"buyImbalanceCount\": " << bar.buyImbalanceCount << ",";
    ss << "\"sellImbalanceCount\": " << bar.sellImbalanceCount << ",";
    ss << "\"delta\": " << bar.delta << ",";
    ss << "\"barDeltaChange\": " << bar.barDeltaChange << ",";
    ss << "\"barHighDelta\": " << bar.barHighDelta << ",";
    ss << "\"barLowDelta\": " << bar.barLowDelta << ",";
    ss << "\"barPOCPrice\": " << bar.barPOCPrice << ",";
    ss << "\"barPOCVol\": " << bar.barPOCVol << ",";    
    ss << "\"signal\": " << bar.signal << ",";
    ss << "\"signalID\": " << bar.signalID << ",";
    ss << "\"signalStatus\": " << (bar.signalStatus ? "true" : "false") << ",";
    ss << "\"priceCurrentDayVwapDiff\": " << bar.priceCurrentDayVwapDiff << ",";
    ss << "\"priceCurrentDayVwapUpperStdDevDiff\": " << bar.priceCurrentDayVwapUpperStdDevDiff << ",";
    ss << "\"priceCurrentDayVwapLowerStdDevDiff\": " << bar.priceCurrentDayVwapLowerStdDevDiff << ",";
    ss << "\"pricePreviousDayVwapDiff\": " << bar.pricePreviousDayVwapDiff << ",";
    ss << "\"priceWeeklyVwapDiff\": " << bar.priceWeeklyVwapDiff << ",";
    ss << "\"priceBBandUpperDiff\": " << bar.priceBBandUpperDiff << ",";
    ss << "\"priceBBandLowerDiff\": " << bar.priceBBandLowerDiff << ",";
    ss << "\"PriceBBandMiddleDiff\": " << bar.PriceBBandMiddleDiff << ",";
    ss << "\"isPriceInVA\": " << (bar.isPriceInVA ? "true" : "false") << ",";
    ss << "\"priceCurrDayVAHDiff\": " << bar.priceCurrDayVAHDiff << ",";
    ss << "\"priceCurrDayVALDiff\": " << bar.priceCurrDayVALDiff << ",";
    ss << "\"pricePrevDayPOCDiff\": " << bar.pricePrevDayPOCDiff << ",";
    ss << "\"pricePrevDayVAHDiff\": " << bar.pricePrevDayVAHDiff << ",";
    ss << "\"pricePrevDayVALDiff\": " << bar.pricePrevDayVALDiff << ",";
    ss << "\"priceIBHighDiff\": " << bar.priceIBHighDiff << ",";
    ss << "\"priceIBLowDiff\": " << bar.priceIBLowDiff << ",";
    ss << "\"pricePrevDayHighDiff\": " << bar.pricePrevDayHighDiff << ",";
    ss << "\"pricePrevDayLowDiff\": " << bar.pricePrevDayLowDiff << ",";
    ss << "\"pricePrevDayCloseDiff\": " << bar.pricePrevDayCloseDiff << ",";
    ss << "\"priceWeekHighDiff\": " << bar.priceWeekHighDiff << ",";
    ss << "\"priceWeekLowDiff\": " << bar.priceWeekLowDiff << ",";
    ss << "\"priceLastSwingHighDiff\": " << bar.priceLastSwingHighDiff << ",";
    ss << "\"priceLastSwingLowDiff\": " << bar.priceLastSwingLowDiff << ",";
    ss << "\"priceLastHVNDiff\": " << bar.priceLastHVNDiff;
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
    ss << "\"deltaZscore20bars\": " << day.deltaZscore20bars << ",";
    ss << "\"cumDelta5barSlope\": " << day.cumDelta5barSlope << ",";
    ss << "\"priceCumDeltaDivergence5bar\": " << day.priceCumDeltaDivergence5bar << ",";
    ss << "\"priceCumDeltaDivergence10bar\": " << day.priceCumDeltaDivergence10bar << ",";
    ss << "\"interactionReversal20bar\": " << day.interactionReversal20bar << ",";
    ss << "\"vwap\": " << day.vwap << ",";
    ss << "\"vwapUpperStdDev1\": " << day.vwapUpperStdDev1 << ",";
    ss << "\"vwapUpperStdDev2\": " << day.vwapUpperStdDev2 << ",";
    ss << "\"vwapLowerStdDev1\": " << day.vwapLowerStdDev1 << ",";
    ss << "\"vwapLowerStdDev2\": " << day.vwapLowerStdDev2 << ",";
    ss << "\"bbMiddle\": " << day.bbMiddle << ",";
    ss << "\"bbUpper\": " << day.bbUpper << ",";
    ss << "\"bbLower\": " << day.bbLower << ",";
    ss << "\"BBandWidth\": " << day.BBandWidth << ",";
    ss << "\"rsi\": " << day.rsi << ",";
    ss << "\"poc\": " << day.poc << ",";
    ss << "\"vah\": " << day.vah << ",";
    ss << "\"val\": " << day.val << ",";
    ss << "\"ibHigh\": " << day.ibHigh << ",";
    ss << "\"ibLow\": " << day.ibLow << ",";
    ss << "\"dayHigh\": " << day.dayHigh << ",";
    ss << "\"dayLow\": " << day.dayLow << ",";
    ss << "\"dayClose\": " << day.dayClose << ",";
    ss << "\"totalVolume\": " << day.totalVolume << ",";
    ss << "\"cumulativeDelta\": " << day.cumulativeDelta << ",";
    ss << "\"lastSwingHigh\": " << day.lastSwingHigh << ",";
    ss << "\"lastSwingLow\": " << day.lastSwingLow << ",";
    ss << "\"lastHighVolumeNode\": " << day.lastHighVolumeNode;
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
    ss << "\"vwap\": " << week.vwap << ",";
    ss << "\"poc\": " << week.poc << ",";
    ss << "\"vah\": " << week.vah << ",";
    ss << "\"val\": " << week.val << ",";
    ss << "\"weekHigh\": " << week.weekHigh << ",";
    ss << "\"weekLow\": " << week.weekLow;
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
