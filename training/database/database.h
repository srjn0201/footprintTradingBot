#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>

struct TickData {
    int id;
    std::string Date;
    std::string Time;
    double Close;
    double AskVolume;
    double BidVolume;
};

class Database {
public:
    Database(const std::string& db_path);
    ~Database();
    std::vector<TickData> fetchData(const std::string& table_name, const std::string& date);

private:
    sqlite3* db;
};

#endif // DATABASE_H