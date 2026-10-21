#include "RailwaySystem.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>

namespace {
const std::string dataDirectory{"data"};

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
}

bool containsDelimiter(const std::string& value) {
    return value.find('|') != std::string::npos;
}

std::string readText(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        if (!value.empty() && !containsDelimiter(value)) return value;
        std::cout << "Enter non-empty text without the | character.\n";
    }
}

int readInt(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        const std::string text = readText(prompt);
        try {
            std::size_t used = 0;
            const long long value = std::stoll(text, &used);
            if (used == text.size() && value >= minimum && value <= maximum) {
                return static_cast<int>(value);
            }
        } catch (const std::exception&) {
            // Invalid numeric text is handled by the message below.
        }
        std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

bool readYesNo(const std::string& prompt) {
    while (true) {
        const std::string answer = lower(readText(prompt));
        if (answer == "y" || answer == "yes") return true;
        if (answer == "n" || answer == "no") return false;
        std::cout << "Please enter y or n.\n";
    }
}

std::string readGender() {
    while (true) {
        const std::string value = lower(readText("Gender (M/F/O): "));
        if (value == "m" || value == "f" || value == "o") {
            return std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(value[0]))));
        }
        std::cout << "Enter M, F, or O.\n";
    }
}

std::string readTravelClass() {
    while (true) {
        const std::string value = lower(readText("Class (SL/3A/2A/1A): "));
        if (value == "sl") return "SL";
        if (value == "3a") return "3A";
        if (value == "2a") return "2A";
        if (value == "1a") return "1A";
        std::cout << "Choose SL, 3A, 2A, or 1A.\n";
    }
}

std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, '|')) fields.push_back(field);
    return fields;
}

bool parseInt(const std::string& text, int& value) {
    try {
        std::size_t used = 0;
        const long long parsed = std::stoll(text, &used);
        if (used != text.size() || parsed < std::numeric_limits<int>::min() ||
            parsed > std::numeric_limits<int>::max()) return false;
        value = static_cast<int>(parsed);
        return true;
    } catch (const std::exception&) { return false; }
}

bool parseLongLong(const std::string& text, long long& value) {
    try {
        std::size_t used = 0;
        value = std::stoll(text, &used);
        return used == text.size();
    } catch (const std::exception&) { return false; }
}

bool parseDouble(const std::string& text, double& value) {
    try {
        std::size_t used = 0;
        value = std::stod(text, &used);
        return used == text.size();
    } catch (const std::exception&) { return false; }
}

void line() { std::cout << "----------------------------------------\n"; }
}  // namespace

RailwaySystem::RailwaySystem() : nextPnr_(500001) {}

void RailwaySystem::rebuildIndexes() {
    trainIndex_.clear();
    passengerIndex_.clear();
    for (std::size_t index = 0; index < trains_.size(); ++index) trainIndex_[trains_[index].number()] = index;
    for (std::size_t index = 0; index < passengers_.size(); ++index) passengerIndex_[passengers_[index].id()] = index;
}

void RailwaySystem::rebuildStations() {
    stations_.clear();
    for (const Train& train : trains_) {
        stations_.insert(train.source());
        stations_.insert(train.destination());
    }
}

void RailwaySystem::rebuildSeatsForTrain(int trainNumber) {
    Train* train = findTrain(trainNumber);
    if (train == nullptr) return;

    std::set<int>& seats = availableSeatNumbers_[trainNumber];
    seats.clear();
    for (int seat = 1; seat <= train->totalSeats(); ++seat) seats.insert(seat);

    for (auto& entry : reservationsByPnr_) {
        Reservation& reservation = entry.second;
        if (reservation.trainNumber() != trainNumber || reservation.status() != ReservationStatus::Confirmed) continue;
        if (reservation.seatNumber() < 1 || reservation.seatNumber() > train->totalSeats() ||
            seats.erase(reservation.seatNumber()) == 0) {
            std::cerr << "Warning: invalid confirmed reservation " << reservation.pnr()
                      << " was cancelled while rebuilding seats.\n";
            reservation.cancel();
        }
    }
    train->setAvailableSeats(static_cast<int>(seats.size()));
}

void RailwaySystem::rebuildDerivedData() {
    rebuildIndexes();
    rebuildStations();
    availableSeatNumbers_.clear();
    waitingLists_.clear();
    for (const Train& train : trains_) rebuildSeatsForTrain(train.number());

    std::map<int, std::vector<long long>> waitingPnrs;
    long long largestPnr = 500000;
    for (const auto& entry : reservationsByPnr_) {
        const Reservation& reservation = entry.second;
        largestPnr = std::max(largestPnr, reservation.pnr());
        if (reservation.status() == ReservationStatus::Waiting && findTrain(reservation.trainNumber()) != nullptr) {
            waitingPnrs[reservation.trainNumber()].push_back(reservation.pnr());
        }
    }
    for (auto& entry : waitingPnrs) {
        std::sort(entry.second.begin(), entry.second.end());
        for (long long pnr : entry.second) waitingLists_[entry.first].push(pnr);
    }
    nextPnr_ = std::max(500001LL, largestPnr + 1);
}

void RailwaySystem::loadData() {
    const auto loadTrains = [this] {
        const std::string path = dataDirectory + "/trains.txt";
        std::ifstream file(path);
        if (!file) { std::cerr << "Warning: could not open " << path << " for reading.\n"; return; }
        std::string record;
        while (std::getline(file, record)) {
            const auto fields = split(record);
            int number = 0, seats = 0;
            if (fields.size() != 8 || !parseInt(fields[0], number) || !parseInt(fields[6], seats) ||
                number <= 0 || seats <= 0 || (fields[7] != "0" && fields[7] != "1") ||
                trainIndex_.count(number) != 0) {
                std::cerr << "Warning: skipped invalid train record.\n";
                continue;
            }
            trains_.emplace_back(number, fields[1], fields[2], fields[3], fields[4], fields[5], seats, fields[7] == "1");
            trainIndex_[number] = trains_.size() - 1;
        }
    };

    const auto loadPassengers = [this] {
        const std::string path = dataDirectory + "/passengers.txt";
        std::ifstream file(path);
        if (!file) { std::cerr << "Warning: could not open " << path << " for reading.\n"; return; }
        std::string record;
        while (std::getline(file, record)) {
            const auto fields = split(record);
            int id = 0, age = 0;
            if (fields.size() != 5 || !parseInt(fields[0], id) || !parseInt(fields[2], age) || id <= 0 ||
                age < 1 || age > 120 || passengerIndex_.count(id) != 0) {
                std::cerr << "Warning: skipped invalid passenger record.\n";
                continue;
            }
            passengers_.emplace_back(id, fields[1], age, fields[3], fields[4]);
            passengerIndex_[id] = passengers_.size() - 1;
        }
    };

    const auto loadReservations = [this] {
        const std::string path = dataDirectory + "/reservations.txt";
        std::ifstream file(path);
        if (!file) { std::cerr << "Warning: could not open " << path << " for reading.\n"; return; }
        std::string record;
        while (std::getline(file, record)) {
            const auto fields = split(record);
            long long pnr = 0;
            int trainNumber = 0, passengerId = 0, seatNumber = 0;
            double fare = 0.0;
            ReservationStatus status;
            if (fields.size() != 7 || !parseLongLong(fields[0], pnr) || !parseInt(fields[1], trainNumber) ||
                !parseInt(fields[2], passengerId) || !parseInt(fields[4], seatNumber) || !parseDouble(fields[6], fare) ||
                !statusFromString(fields[5], status) || pnr < 500001 || trainIndex_.count(trainNumber) == 0 ||
                passengerIndex_.count(passengerId) == 0 || reservationsByPnr_.count(pnr) != 0) {
                std::cerr << "Warning: skipped invalid reservation record.\n";
                continue;
            }
            reservationsByPnr_.emplace(pnr, Reservation(pnr, trainNumber, passengerId, fields[3], seatNumber, status, fare));
        }
    };

    loadTrains();
    loadPassengers();
    loadReservations();
    rebuildDerivedData();
    std::cout << "Data loaded: " << trains_.size() << " train(s), " << passengers_.size()
              << " passenger(s), " << reservationsByPnr_.size() << " reservation(s).\n";
}

bool RailwaySystem::saveData() const {
    std::ofstream trainFile(dataDirectory + "/trains.txt", std::ios::trunc);
    std::ofstream passengerFile(dataDirectory + "/passengers.txt", std::ios::trunc);
    std::ofstream reservationFile(dataDirectory + "/reservations.txt", std::ios::trunc);
    if (!trainFile || !passengerFile || !reservationFile) {
        std::cerr << "Could not open one or more data files for writing.\n";
        return false;
    }

    for (const Train& train : trains_) {
        trainFile << train.number() << '|' << train.name() << '|' << train.source() << '|'
                  << train.destination() << '|' << train.departureTime() << '|' << train.arrivalTime() << '|'
                  << train.totalSeats() << '|' << (train.isActive() ? 1 : 0) << '\n';
    }
    for (const Passenger& passenger : passengers_) {
        passengerFile << passenger.id() << '|' << passenger.name() << '|' << passenger.age() << '|'
                      << passenger.gender() << '|' << passenger.phone() << '\n';
    }
    reservationFile << std::fixed << std::setprecision(2);
    for (const auto& entry : reservationsByPnr_) {
        const Reservation& reservation = entry.second;
        reservationFile << reservation.pnr() << '|' << reservation.trainNumber() << '|'
                        << reservation.passengerId() << '|' << reservation.travelClass() << '|'
                        << reservation.seatNumber() << '|' << toString(reservation.status()) << '|'
                        << reservation.fare() << '\n';
    }
    if (!trainFile || !passengerFile || !reservationFile) {
        std::cerr << "A write error occurred while saving data.\n";
        return false;
    }
    return true;
}

Train* RailwaySystem::findTrain(int trainNumber) {
    const auto found = trainIndex_.find(trainNumber);
    return found == trainIndex_.end() ? nullptr : &trains_[found->second];
}

const Train* RailwaySystem::findTrain(int trainNumber) const {
    const auto found = trainIndex_.find(trainNumber);
    return found == trainIndex_.end() ? nullptr : &trains_[found->second];
}

Passenger* RailwaySystem::findPassenger(int passengerId) {
    const auto found = passengerIndex_.find(passengerId);
    return found == passengerIndex_.end() ? nullptr : &passengers_[found->second];
}

const Passenger* RailwaySystem::findPassenger(int passengerId) const {
    const auto found = passengerIndex_.find(passengerId);
    return found == passengerIndex_.end() ? nullptr : &passengers_[found->second];
}

void RailwaySystem::showDashboard() const {
    const int activeTrains = static_cast<int>(std::count_if(trains_.begin(), trains_.end(),
        [](const Train& train) { return train.isActive(); }));
    const int confirmed = static_cast<int>(std::count_if(reservationsByPnr_.begin(), reservationsByPnr_.end(),
        [](const auto& entry) { return entry.second.status() == ReservationStatus::Confirmed; }));
    const int waiting = static_cast<int>(std::count_if(reservationsByPnr_.begin(), reservationsByPnr_.end(),
        [](const auto& entry) { return entry.second.status() == ReservationStatus::Waiting; }));
    const int available = std::accumulate(trains_.begin(), trains_.end(), 0,
        [](int total, const Train& train) { return total + (train.isActive() ? train.availableSeats() : 0); });

    std::cout << "\n========================================\n"
              << "                RAILFLOW\n"
              << "========================================\n"
              << "Total Trains          : " << trains_.size() << '\n'
              << "Active Trains         : " << activeTrains << '\n'
              << "Total Passengers      : " << passengers_.size() << '\n'
              << "Total Reservations    : " << reservationsByPnr_.size() << '\n'
              << "Confirmed Tickets     : " << confirmed << '\n'
              << "Waiting List          : " << waiting << '\n'
              << "Available Seats       : " << available << '\n'
              << "Stations              : " << stations_.size() << '\n'
              << "========================================\n";
}

void RailwaySystem::addTrain() {
    std::cout << "\n--- Add Train ---\n";
    const int number = readInt("Train number: ", 1, 999999);
    if (findTrain(number) != nullptr) { std::cout << "A train with that number already exists.\n"; return; }
    const std::string name = readText("Train name: ");
    const std::string source = readText("Source: ");
    const std::string destination = readText("Destination: ");
    if (lower(source) == lower(destination)) { std::cout << "Source and destination must be different.\n"; return; }
    const std::string departure = readText("Departure time (HH:MM): ");
    const std::string arrival = readText("Arrival time (HH:MM): ");
    const int seats = readInt("Total seats: ", 1, 100000);
    trains_.emplace_back(number, name, source, destination, departure, arrival, seats);
    trainIndex_[number] = trains_.size() - 1;
    rebuildStations();
    rebuildSeatsForTrain(number);
    std::cout << "Train added successfully.\n";
}

void RailwaySystem::viewAndSortTrains() const {
    if (trains_.empty()) { std::cout << "No trains have been added.\n"; return; }
    std::cout << "\nSort by:\n1. Train number\n2. Train name\n3. Departure time\n4. Available seats\n";
    const int choice = readInt("Choice: ", 1, 4);
    std::vector<const Train*> results;
    for (const Train& train : trains_) results.push_back(&train);
    std::sort(results.begin(), results.end(), [choice](const Train* left, const Train* right) {
        if (choice == 1) return left->number() < right->number();
        if (choice == 2) return lower(left->name()) < lower(right->name());
        if (choice == 3) return left->departureTime() < right->departureTime();
        return left->availableSeats() > right->availableSeats();
    });
    std::cout << "\n--- Trains ---\n";
    for (const Train* train : results) { train->display(); line(); }
}

void RailwaySystem::searchTrains() const {
    std::cout << "\nSearch by:\n1. Source and destination\n2. Train number\n3. Train name\n";
    const int choice = readInt("Choice: ", 1, 3);
    std::vector<const Train*> results;
    if (choice == 1) {
        const std::string source = lower(readText("From: "));
        const std::string destination = lower(readText("To: "));
        if (source == destination) { std::cout << "Source and destination must be different.\n"; return; }
        for (const Train& train : trains_) {
            if (lower(train.source()) == source && lower(train.destination()) == destination) results.push_back(&train);
        }
    } else if (choice == 2) {
        const int number = readInt("Train number: ", 1, 999999);
        const auto found = std::find_if(trains_.begin(), trains_.end(), [number](const Train& train) {
            return train.number() == number;
        });
        if (found != trains_.end()) results.push_back(&*found);
    } else {
        const std::string query = lower(readText("Name text: "));
        for (const Train& train : trains_) {
            if (lower(train.name()).find(query) != std::string::npos) results.push_back(&train);
        }
    }
    std::cout << "\n--- Search Results ---\n";
    if (results.empty()) { std::cout << "No matching trains found.\n"; return; }
    for (const Train* train : results) { train->display(); line(); }
}

void RailwaySystem::updateTrain() {
    std::cout << "\n--- Update Train ---\n";
    const int number = readInt("Train number: ", 1, 999999);
    Train* train = findTrain(number);
    if (train == nullptr) { std::cout << "Train not found.\n"; return; }
    const std::string name = readText("New train name: ");
    const std::string source = readText("New source: ");
    const std::string destination = readText("New destination: ");
    if (lower(source) == lower(destination)) { std::cout << "Source and destination must be different. Update cancelled.\n"; return; }
    const std::string departure = readText("New departure time (HH:MM): ");
    const std::string arrival = readText("New arrival time (HH:MM): ");
    const int seats = readInt("New total seats: ", 1, 100000);
    int largestAssignedSeat = 0;
    for (const auto& entry : reservationsByPnr_) {
        const Reservation& reservation = entry.second;
        if (reservation.trainNumber() == number && reservation.status() == ReservationStatus::Confirmed) {
            largestAssignedSeat = std::max(largestAssignedSeat, reservation.seatNumber());
        }
    }
    if (seats < largestAssignedSeat) {
        std::cout << "Cannot reduce capacity below already assigned seat " << largestAssignedSeat << ".\n";
        return;
    }
    train->update(name, source, destination, departure, arrival, seats);
    rebuildStations();
    rebuildSeatsForTrain(number);
    promoteWaitingPassengers(number);
    std::cout << "Train updated successfully.\n";
}

void RailwaySystem::deactivateTrain() {
    std::cout << "\n--- Deactivate Train ---\n";
    const int number = readInt("Train number: ", 1, 999999);
    Train* train = findTrain(number);
    if (train == nullptr) { std::cout << "Train not found.\n"; return; }
    if (!train->isActive()) { std::cout << "This train is already inactive.\n"; return; }
    train->deactivate();
    std::cout << "Train deactivated. Existing records are kept, but no new tickets can be booked.\n";
}

void RailwaySystem::addPassenger() {
    std::cout << "\n--- Add Passenger ---\n";
    const int id = readInt("Passenger ID: ", 1, 999999999);
    if (findPassenger(id) != nullptr) { std::cout << "A passenger with that ID already exists.\n"; return; }
    const std::string name = readText("Name: ");
    const int age = readInt("Age: ", 1, 120);
    const std::string gender = readGender();
    const std::string phone = readText("Phone: ");
    passengers_.emplace_back(id, name, age, gender, phone);
    passengerIndex_[id] = passengers_.size() - 1;
    std::cout << "Passenger added successfully.\n";
}

void RailwaySystem::viewPassengers() const {
    if (passengers_.empty()) { std::cout << "No passengers have been added.\n"; return; }
    std::cout << "\n--- Passengers ---\n";
    for (const Passenger& passenger : passengers_) { passenger.display(); line(); }
}

void RailwaySystem::searchPassengers() const {
    std::cout << "\nSearch by:\n1. Passenger ID\n2. Name\n3. Phone\n";
    const int choice = readInt("Choice: ", 1, 3);
    std::vector<const Passenger*> results;
    if (choice == 1) {
        const int id = readInt("Passenger ID: ", 1, 999999999);
        const auto found = std::find_if(passengers_.begin(), passengers_.end(), [id](const Passenger& passenger) {
            return passenger.id() == id;
        });
        if (found != passengers_.end()) results.push_back(&*found);
    } else {
        const std::string query = lower(readText("Search text: "));
        for (const Passenger& passenger : passengers_) {
            const std::string& field = choice == 2 ? passenger.name() : passenger.phone();
            if (lower(field).find(query) != std::string::npos) results.push_back(&passenger);
        }
    }
    std::cout << "\n--- Search Results ---\n";
    if (results.empty()) { std::cout << "No matching passengers found.\n"; return; }
    for (const Passenger* passenger : results) { passenger->display(); line(); }
}

long long RailwaySystem::generatePnr() {
    while (reservationsByPnr_.count(nextPnr_) != 0) ++nextPnr_;
    return nextPnr_++;
}

double RailwaySystem::calculateFare(const std::string& travelClass) const {
    if (travelClass == "SL") return 450.0;
    if (travelClass == "3A") return 900.0;
    if (travelClass == "2A") return 1400.0;
    return 2200.0;
}

void RailwaySystem::displayReservation(const Reservation& reservation) const {
    reservation.display();
    const Train* train = findTrain(reservation.trainNumber());
    const Passenger* passenger = findPassenger(reservation.passengerId());
    if (train != nullptr) std::cout << "Train Name   : " << train->name() << '\n';
    if (passenger != nullptr) std::cout << "Passenger    : " << passenger->name() << '\n';
}

void RailwaySystem::bookTicket() {
    std::cout << "\n--- Book Ticket ---\n";
    const int trainNumber = readInt("Train number: ", 1, 999999);
    Train* train = findTrain(trainNumber);
    if (train == nullptr) { std::cout << "Train not found.\n"; return; }
    if (!train->isActive()) { std::cout << "This train is inactive and cannot accept bookings.\n"; return; }
    const int passengerId = readInt("Passenger ID: ", 1, 999999999);
    if (findPassenger(passengerId) == nullptr) { std::cout << "Passenger not found. Add the passenger first.\n"; return; }
    const std::string travelClass = readTravelClass();
    const long long pnr = generatePnr();
    const double fare = calculateFare(travelClass);
    std::set<int>& seats = availableSeatNumbers_[trainNumber];

    if (!seats.empty()) {
        const int seat = *seats.begin();
        seats.erase(seats.begin());
        reservationsByPnr_.emplace(pnr, Reservation(pnr, trainNumber, passengerId, travelClass, seat,
                                                    ReservationStatus::Confirmed, fare));
        train->setAvailableSeats(static_cast<int>(seats.size()));
        std::cout << "Ticket confirmed. PNR: " << pnr << ", seat: " << seat << ".\n";
    } else {
        std::cout << "No seats are available.\n";
        if (!readYesNo("Add passenger to the waiting list? (y/n): ")) {
            std::cout << "Booking not created.\n";
            return;
        }
        reservationsByPnr_.emplace(pnr, Reservation(pnr, trainNumber, passengerId, travelClass, -1,
                                                    ReservationStatus::Waiting, fare));
        waitingLists_[trainNumber].push(pnr);
        std::cout << "Added to waiting list. PNR: " << pnr << ".\n";
    }
}

void RailwaySystem::removeFromWaitingList(int trainNumber, long long pnr) {
    auto list = waitingLists_.find(trainNumber);
    if (list == waitingLists_.end()) return;
    std::vector<long long> pending;
    while (!list->second.empty()) {
        pending.push_back(list->second.front());
        list->second.pop();
    }
    pending.erase(std::remove_if(pending.begin(), pending.end(),
                                 [pnr](long long current) { return current == pnr; }),
                  pending.end());
    std::queue<long long> kept;
    for (long long current : pending) kept.push(current);
    list->second.swap(kept);
}

void RailwaySystem::promoteWaitingPassengers(int trainNumber) {
    Train* train = findTrain(trainNumber);
    if (train == nullptr || !train->isActive()) return;
    std::set<int>& seats = availableSeatNumbers_[trainNumber];
    std::queue<long long>& list = waitingLists_[trainNumber];
    while (!seats.empty() && !list.empty()) {
        const long long pnr = list.front();
        list.pop();
        auto reservation = reservationsByPnr_.find(pnr);
        if (reservation == reservationsByPnr_.end() || reservation->second.status() != ReservationStatus::Waiting ||
            reservation->second.trainNumber() != trainNumber) continue;
        const int seat = *seats.begin();
        seats.erase(seats.begin());
        reservation->second.confirm(seat);
        std::cout << "Waiting-list promotion: PNR " << pnr << " is now confirmed with seat " << seat << ".\n";
    }
    train->setAvailableSeats(static_cast<int>(seats.size()));
}

void RailwaySystem::cancelTicket() {
    std::cout << "\n--- Cancel Ticket ---\n";
    const long long pnr = static_cast<long long>(readInt("PNR: ", 500001, 999999999));
    auto found = reservationsByPnr_.find(pnr);
    if (found == reservationsByPnr_.end()) { std::cout << "Invalid PNR.\n"; return; }
    Reservation& reservation = found->second;
    if (reservation.status() == ReservationStatus::Cancelled) { std::cout << "This ticket is already cancelled.\n"; return; }
    Train* train = findTrain(reservation.trainNumber());
    if (train == nullptr) { std::cout << "Reservation refers to a missing train; cancellation stopped safely.\n"; return; }

    if (reservation.status() == ReservationStatus::Waiting) {
        removeFromWaitingList(reservation.trainNumber(), pnr);
        reservation.cancel();
        std::cout << "Waiting-list ticket cancelled.\n";
        return;
    }

    availableSeatNumbers_[reservation.trainNumber()].insert(reservation.seatNumber());
    reservation.cancel();
    train->setAvailableSeats(static_cast<int>(availableSeatNumbers_[reservation.trainNumber()].size()));
    std::cout << "Confirmed ticket cancelled. Seat released.\n";
    promoteWaitingPassengers(reservation.trainNumber());
}

void RailwaySystem::searchPnr() const {
    std::cout << "\n--- Search PNR ---\n";
    const long long pnr = static_cast<long long>(readInt("PNR: ", 500001, 999999999));
    const auto found = reservationsByPnr_.find(pnr);
    if (found == reservationsByPnr_.end()) { std::cout << "Invalid PNR.\n"; return; }
    displayReservation(found->second);
}

void RailwaySystem::viewWaitingList() const {
    std::cout << "\n--- Waiting List ---\n";
    const int trainNumber = readInt("Train number: ", 1, 999999);
    if (findTrain(trainNumber) == nullptr) { std::cout << "Train not found.\n"; return; }
    const auto found = waitingLists_.find(trainNumber);
    if (found == waitingLists_.end() || found->second.empty()) { std::cout << "No passengers are waiting for this train.\n"; return; }
    std::queue<long long> list = found->second;  // Copy lets us inspect a queue without changing it.
    int position = 1;
    while (!list.empty()) {
        const long long pnr = list.front();
        list.pop();
        const auto reservation = reservationsByPnr_.find(pnr);
        if (reservation == reservationsByPnr_.end() || reservation->second.status() != ReservationStatus::Waiting) continue;
        std::cout << position++ << ". PNR " << pnr;
        const Passenger* passenger = findPassenger(reservation->second.passengerId());
        if (passenger != nullptr) std::cout << " - " << passenger->name();
        std::cout << '\n';
    }
}

void RailwaySystem::viewStations() const {
    if (stations_.empty()) { std::cout << "No stations are available yet.\n"; return; }
    std::cout << "\n--- Stations (unique, alphabetical) ---\n";
    for (const std::string& station : stations_) std::cout << "- " << station << '\n';
}
