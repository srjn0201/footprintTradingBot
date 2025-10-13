#include <string>
#include <vector>
#include <sqlite3.h>
#include <iostream>
#include "database.h"
#include "../dataStructure.h"


std::vector<TickData> fetchData(const std::string& database_path, const std::string& table_name, const Date date) {
    std::vector<TickData> result;
    sqlite3* db;
    
    // Open database
    int rc = sqlite3_open(database_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    // Format date string YYYY-MM-DD
    char date_str[11];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", date.y, date.m, date.d);

    // Prepare SQL statement with proper date filtering
    std::string sql = "SELECT id, DateTime, Price, AskVolume, BidVolume FROM " + 
                     table_name + 
                     " WHERE strftime('%Y-%m-%d', DateTime) = ?";
    
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }

    // Bind date parameter
    sqlite3_bind_text(stmt, 1, date_str, -1, SQLITE_STATIC);

    // Execute query and fetch results
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TickData tick;
        tick.id = sqlite3_column_int(stmt, 0);
        tick.DateTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        tick.Price = sqlite3_column_double(stmt, 2);
        tick.AskVolume = sqlite3_column_double(stmt, 3);
        tick.BidVolume = sqlite3_column_double(stmt, 4);
        result.push_back(tick);
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}
