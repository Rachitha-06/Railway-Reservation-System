#pragma once

#include <string>

class Train {
public:
    Train(int number, std::string name, std::string source, std::string destination,
          std::string departureTime, std::string arrivalTime, int totalSeats,
          bool active = true);

    int number() const;
    const std::string& name() const;
    const std::string& source() const;
    const std::string& destination() const;
    const std::string& departureTime() const;
    const std::string& arrivalTime() const;
    int totalSeats() const;
    int availableSeats() const;
    bool isActive() const;

    void update(std::string newName, std::string newSource, std::string newDestination,
                std::string newDepartureTime, std::string newArrivalTime, int newTotalSeats);
    void setAvailableSeats(int seats);
    void deactivate();
    void display() const;

private:
    int trainNumber_;
    std::string name_;
    std::string source_;
    std::string destination_;
    std::string departureTime_;
    std::string arrivalTime_;
    int totalSeats_;
    int availableSeats_;
    bool active_;
};
