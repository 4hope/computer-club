#ifndef EVENT_H
#define EVENT_H

#include "clock_time.h"

#include <iostream>
#include <optional>
#include <sstream>

class Event {
public:
    Event(Time, std::optional<int>);
    ~Event() = default;

    friend std::istringstream &operator>>(std::istringstream&, Event&);
    friend std::ostream &operator<<(std::ostream&, const Event&);

    Time get_time();
    int get_id();

protected:
    Time time_;
    std::optional<int> id_;
};


class OutgoingEvent : public Event {
public:
    OutgoingEvent();
    ~OutgoingEvent() = default;

    friend std::istringstream &operator>>(std::istringstream&, OutgoingEvent&);
    friend std::ostream &operator<<(std::ostream&, OutgoingEvent&);

    static bool is_valid_client_name(const std::string& s);

    int get_table_number();
    std::string get_client_name();

private:
    std::string client_name_;
    std::optional<int> table_number_;
};


class IncomingEvent : public Event {
public:
    IncomingEvent(Time, int, std::optional<std::string> = std::nullopt,
                  std::optional<std::string> = std::nullopt, std::optional<int> = std::nullopt);
    ~IncomingEvent() = default;

    friend std::ostream &operator<<(std::ostream&, IncomingEvent&);

    int get_table_number();
    std::string get_client_name();

private:
    std::optional<std::string> client_name_;
    std::optional<std::string> fault_;
    std::optional<int> table_number_;
};

#endif