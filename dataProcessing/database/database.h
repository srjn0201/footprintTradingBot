#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include "../dataStructure.h"

struct TickData {
    int id;
    std::string DateTime;
    double Price;
    double AskVolume;
    double BidVolume;
};


std::vector<TickData> fetchData(const std::string& database_path, const std::string& table_name, const Date date);

TickData fetchFirstTick(const std::string& database_path, const std::string& table_name, const Date date);


#endif // DATABASE_H