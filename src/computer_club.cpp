#include "computer_club.h"
#include "utils.h"

#include <iomanip>
#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// CLIENT
Client::Client(std::string client_name, Time start_time, std::optional<int> table_number) : 
    client_name_(client_name), start_time_(start_time), table_number_(table_number) { }

optional<int> Client::get_table_number() { return table_number_; }
string Client::get_client_name() const { return client_name_; }
Time &Client::get_start_time() {return start_time_; }

void Client::set_table_number(int new_table_number) { table_number_ = new_table_number; }
void Client::set_start_time(Time time) { start_time_ = time; }

bool operator<(const Client &a, const Client &b) {
    return a.client_name_ < b.client_name_;
}


// TABLE
Table::Table(int number) : number_(number) {
    free_ = true;
    revenue_ = 0;
    use_time_ = Time();
}

bool Table::is_free() { return free_; }
void Table::set_free() { free_ = true; }
void Table::set_busy() { free_ = false; }

void Table::add_revenue(int revenue) { revenue_ += revenue; }
void Table::add_time(Time time) { use_time_ = use_time_ + time; }

int Table::get_number() { return number_; }

ostream &operator<<(std::ostream& f, const Table& table) {
    f << table.number_ << " " << table.revenue_ << " " << table.use_time_ << '\n';
    return f;
}

bool operator<(const Table &a, const Table &b) {
    return a.free_ < b.free_;
}

void Table::cout_money_time(Time time, int table_count) {
    add_time(time);

    int cur_revenue = (time.get_minutes() > 0 ? time.get_hours() + 1 : time.get_hours()) * table_count;
    add_revenue(cur_revenue);
}


// COMPUTER CLUB
ifstream &operator>>(std::ifstream& f, ComputerClub& club) {
    if (!read_single_value_line(f, club.table_count_)) {
        f.setstate(ios::failbit);
        return f;
    }

    if (!read_two_values_line(f, club.work_start_, club.work_end_)) {
        f.setstate(ios::failbit);
        return f;
    }

    if (!read_single_value_line(f, club.hour_cost_)) {
        f.setstate(ios::failbit);
        return f;
    }

    Table fake_table = Table(-1);
    fake_table.set_busy();
    club.tables_.push_back(fake_table);

    for (int i = 0; i < club.table_count_; ++i) {
        club.tables_.push_back(Table(i + 1));
    }
    
    return f;
}

ostream &operator<<(ostream& f, const ComputerClub& club) {
    for (auto client : club.clients_) {
        IncomingEvent event = IncomingEvent(club.work_end_, 11, client.get_client_name());
        f << event;
    }

    f << club.work_end_ << '\n';
    for (int i = 1; i <= club.table_count_; ++i) {
        f << club.tables_[i];
    }

    return f;
}

optional<int> ComputerClub::remove_table_free(Client& client, Time time) {
    optional<int> table_number = client.get_table_number();
    if (table_number) {
        Table& table = tables_[table_number.value()];
        table.set_free();
        table.cout_money_time(time - client.get_start_time(), hour_cost_);
    }
    return table_number;
}

optional<int> ComputerClub::delete_client(Client& client, Time time) {
    clients_.erase(client);
    return remove_table_free(client, time);
}

optional<reference_wrapper<Client>> ComputerClub::is_here(string client_name) {
    auto it = find_if(clients_.begin(), clients_.end(),
                           [&client_name](const Client &c)
                           { return c.get_client_name() == client_name; });

    if (it != clients_.end()) {
        return ref(const_cast<Client &>(*it));
    }
    return nullopt;
}

bool ComputerClub::has_free_tables() {
    for (auto table : tables_) {
        if (table.is_free()) return true;
    }
    return false;
}

bool ComputerClub::is_closed(Time time) {
    return time < work_start_ || work_end_ < time;
}

void ComputerClub::check_event(OutgoingEvent& event) {
    switch (event.get_id()) {
        case 1:
            if (is_here(event.get_client_name())) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 13, nullopt, "YouShallNotPass");
                cout << fault;
            }
            else if (is_closed(event.get_time())) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 13, nullopt, "NotOpenYet");
                cout << fault;
            }
            else {
                clients_.insert(Client(event.get_client_name(), event.get_time()));
            }
            break;
        case 2: {
            auto cl = is_here(event.get_client_name());
            if (!cl) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 13, nullopt, "ClientUnknown");
                cout << fault;
            }
            else if (!tables_[event.get_table_number()].is_free()) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 13, nullopt, "PlaceIsBusy");
                cout << fault;
            }
            else if (cl->get().get_table_number()) {
                Client &client = cl->get();
                tables_[client.get_table_number().value()].set_free();
                client.set_table_number(event.get_table_number());
                client.set_start_time(event.get_time());
                tables_[event.get_table_number()].set_busy();
            }
            else {
                Client &client = cl->get();
                client.set_table_number(event.get_table_number());
                client.set_start_time(event.get_time());
                tables_[event.get_table_number()].set_busy();
            }
            break;
        }
        case 3:
            if (has_free_tables()) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 13, nullopt, "ICanWaitNoLonger!");
                cout << fault;
            }
            else if (static_cast<int>(events_.size()) > table_count_) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 11, event.get_client_name());
                cout << fault;
            }
            else events_.push_back(event);
            break;
        case 4: {
            auto is_client = is_here(event.get_client_name());
            if (!is_client) {
                IncomingEvent fault = IncomingEvent(event.get_time(), 13, nullopt, "ClientUnknown");
                cout << fault;
            }
            else {
                Client &client = is_client->get();
                optional<int> table_number = delete_client(client, event.get_time());
                if (table_number && events_.size() >= 1) {
                    OutgoingEvent new_event = events_.front();
                    events_.pop_front();

                    auto new_client_link = is_here(new_event.get_client_name());
                    Client& new_client = new_client_link->get();
                    new_client.set_table_number(table_number.value());
                    new_client.set_start_time(event.get_time());

                    Table& table = tables_[table_number.value()];
                    table.set_busy();

                    IncomingEvent e = IncomingEvent(event.get_time(), 12, new_event.get_client_name(), nullopt, table_number);
                    cout << e;
                }
            }
            break;
        }
        default:
            break;
    }
}

bool ComputerClub::simulate(ifstream& f) {
    vector<OutgoingEvent> outgoing;

    string line, fault;
    while (getline(f, line)) {
        istringstream ss(line);
        OutgoingEvent ev;
        ss >> ev;

        bool flag = false;
        if (ss.fail() || (ss >> fault))
            flag = true;
        else if (!outgoing.empty() && ev.get_time() < outgoing.back().get_time())
            flag = true;
        else if (ev.get_id() == 2) {
            if (ev.get_table_number() > table_count_ || ev.get_table_number() < 1)
                flag = true;
            else if (count(line.begin(), line.end(), ' ') != 3)
                flag = true;
        }
        else if (count(line.begin(), line.end(), ' ') != 2)
            flag = true;

        if (flag) {
            cout << line << endl;
            f.setstate(ios::failbit);
            return false;
        }

        outgoing.push_back(ev);
    }

    cout << work_start_ << endl;

    for (auto ev : outgoing) {
        cout << ev;
        check_event(ev);
    }

    for (auto client : clients_) {
        remove_table_free(client, work_end_);
    }

    return true;
}

int ComputerClub::get_table_count() { return table_count_; }
