#include "Train.h"

#include <iostream>
#include <utility>

Train::Train(int number, std::string name, std::string source, std::string destination,
             std::string departureTime, std::string arrivalTime, int totalSeats, bool active)
    : trainNumber_(number), name_(std::move(name)), source_(std::move(source)),
      destination_(std::move(destination)), departureTime_(std::move(departureTime)),
      arrivalTime_(std::move(arrivalTime)), totalSeats_(totalSeats),
      availableSeats_(totalSeats), active_(active) {}

int Train::number() const { return trainNumber_; }
const std::string& Train::name() const { return name_; }
const std::string& Train::source() const { return source_; }
const std::string& Train::destination() const { return destination_; }
const std::string& Train::departureTime() const { return departureTime_; }
const std::string& Train::arrivalTime() const { return arrivalTime_; }
int Train::totalSeats() const { return totalSeats_; }
int Train::availableSeats() const { return availableSeats_; }
bool Train::isActive() const { return active_; }

void Train::update(std::string newName, std::string newSource, std::string newDestination,
                   std::string newDepartureTime, std::string newArrivalTime, int newTotalSeats) {
    name_ = std::move(newName);
    source_ = std::move(newSource);
    destination_ = std::move(newDestination);
    departureTime_ = std::move(newDepartureTime);
    arrivalTime_ = std::move(newArrivalTime);
    totalSeats_ = newTotalSeats;
}

void Train::setAvailableSeats(int seats) { availableSeats_ = seats; }
void Train::deactivate() { active_ = false; }

void Train::display() const {
    std::cout << "Train Number : " << trainNumber_ << '\n'
              << "Name         : " << name_ << '\n'
              << "Route        : " << source_ << " -> " << destination_ << '\n'
              << "Departure    : " << departureTime_ << '\n'
              << "Arrival      : " << arrivalTime_ << '\n'
              << "Seats        : " << availableSeats_ << " / " << totalSeats_ << " available\n"
              << "Status       : " << (active_ ? "Active" : "Inactive") << '\n';
}
