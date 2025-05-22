#ifndef CLOCK_TIME_H
#define CLOCK_TIME_H

#include <iostream>
#include <optional>
#include <sstream>

class Time {
public:
    Time() = default;
    Time(int, int);
    ~Time() = default;

    friend std::istringstream &operator>>(std::istringstream&, Time&);
    friend std::ostream &operator<<(std::ostream&, const Time&);

    Time operator+(const Time &);
    Time operator-(const Time &);

    friend bool operator<(const Time&, const Time&);

    int get_hours();
    int get_minutes();

private:
    int hours_;
    int minutes_;
};

#endif