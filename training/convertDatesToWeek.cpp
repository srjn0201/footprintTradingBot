#include <vector>
#include <ctime>
#include <iostream>
#include <array>
#include <iomanip>
#include <chrono>
#include "dataStructure.h"

// Forward declarations
int getDayOfWeek(Date date);
Date addDay(Date date);
Date subtractDay(Date date);
bool operator<=(const Date &a, const Date &b);
bool operator>=(const Date &a, const Date &b);
bool operator<(const Date &a, const Date &b);

// Convert function updated to use Date
std::vector<weekVector> convertDatesToWeeks(Date startDate, Date endDate) {
	std::vector<weekVector> weeksVector;

	// Find the first Monday from or before the start date
	Date currentDate = startDate;
	while (getDayOfWeek(currentDate) != 1) { // 1 is Monday
		currentDate = subtractDay(currentDate);
	}

	int weekCount = 1;
	// Create weeks until we pass the end date
	while (currentDate <= endDate) {
		weekVector currentWeek;
		currentWeek.weekNumber = weekCount++;
		currentWeek.startDate = currentDate;

		// Fill each week with 7 days
		for (int i = 0; i < 7; i++) {
			if (currentDate >= startDate && currentDate <= endDate) {
				currentWeek.days[i] = {getDayOfWeek(currentDate), currentDate};
			}
			if (i == 6) {
				currentWeek.endDate = currentDate;
			}
			currentDate = addDay(currentDate);
		}

		weeksVector.push_back(currentWeek);
	}

	return weeksVector;
}

// Helper utilities

static std::tm to_tm(const Date &dt) {
	std::tm tm = {};
	tm.tm_year = dt.y - 1900;
	tm.tm_mon  = dt.m - 1;
	tm.tm_mday = dt.d;
	// use midday to reduce DST issues
	tm.tm_hour = 12;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	return tm;
}

static Date from_tm(const std::tm &tm) {
	return Date{tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday};
}

int getDayOfWeek(Date date) { // 1 = Monday, ..., 7 = Sunday
	std::tm tm = to_tm(date);
	std::time_t t = std::mktime(&tm); // normalizes tm
	std::tm *res = std::localtime(&t);
	int w = res->tm_wday; // 0 = Sunday, 1 = Monday, ...
	return (w == 0) ? 7 : w;
}

Date addDay(Date date) {
	std::tm tm = to_tm(date);
	std::time_t t = std::mktime(&tm);
	t += 24 * 60 * 60;
	std::tm *res = std::localtime(&t);
	return from_tm(*res);
}

Date subtractDay(Date date) {
	std::tm tm = to_tm(date);
	std::time_t t = std::mktime(&tm);
	t -= 24 * 60 * 60;
	std::tm *res = std::localtime(&t);
	return from_tm(*res);
}

// comparison operators
bool operator<=(const Date &a, const Date &b) {
	if (a.y != b.y) return a.y < b.y;
	if (a.m != b.m) return a.m < b.m;
	return a.d <= b.d;
}
bool operator>=(const Date &a, const Date &b) {
	return !(a < b);
}
bool operator<(const Date &a, const Date &b) {
	if (a.y != b.y) return a.y < b.y;
	if (a.m != b.m) return a.m < b.m;
	return a.d < b.d;
}
