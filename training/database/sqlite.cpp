#include "database.h"
#include <iostream>
#include <vector>

Database::Database(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

static int callback(void* data, int argc, char** argv, char** azColName) {
    std::vector<TickData>* rows = static_cast<std::vector<TickData>*>(data);
    TickData row;
    for (int i = 0; i < argc; i++) {
        std::string colName(azColName[i]);
        if (colName == "id") {
            row.id = argv[i] ? std::stoi(argv[i]) : 0;
        } else if (colName == "Date") {
            row.Date = argv[i] ? argv[i] : "";
        } else if (colName == "Time") {
            row.Time = argv[i] ? argv[i] : "";
        } else if (colName == "Close") {
            row.Close = argv[i] ? std::stod(argv[i]) : 0.0;
        } else if (colName == "AskVolume") {
            row.AskVolume = argv[i] ? std::stod(argv[i]) : 0.0;
        } else if (colName == "BidVolume") {
            row.BidVolume = argv[i] ? std::stod(argv[i]) : 0.0;
        }
    }
    rows->push_back(row);
    return 0;
}

std::vector<TickData> Database::fetchData(const std::string& table_name, const std::string& date) {
    std::vector<TickData> data;
    if (!db) {
        return data;
    }

    std::string query = "SELECT id, Date, Time, Close, AskVolume, BidVolume FROM " + table_name + " WHERE Date = '" + date + "';";
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, query.c_str(), callback, &data, &zErrMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    return data;
}

