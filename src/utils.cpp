#include "clock_time.h"

#include <sstream>
#include <iostream>
#include <charconv>
#include <fstream>
#include <algorithm>

using namespace std;

istringstream &operator>>(istringstream& f, int& num) {
    string number;
    f >> number;

    auto [ptr, ec] = from_chars(number.data(), number.data() + number.size(), num);

    if (ec != errc() || ptr != number.data() + number.size()) {
        f.setstate(ios::failbit);
    }

    return f;
}

bool read_single_value_line(ifstream& f, int& value) {
    string line, extra;
    if (!getline(f, line)) return false;

    istringstream ss(line);
    if (!(ss >> value) || (ss >> extra)) {
        cout << line << endl;
        return false;
    }

    if (value <= 0) {
        cout << line << endl;
        return false;
    }

    return true;
}

bool read_two_values_line(ifstream& f, Time& first, Time& second) {
    string line, extra;
    if (!getline(f, line)) return false;

    istringstream ss(line);
    if (!(ss >> first >> second) || (ss >> extra)) {
        cout << line << endl;
        return false;
    }

    if (count(line.begin(), line.end(), ' ') != 1) {
        cout << line << endl;
        return false;
    }

    return true;
}