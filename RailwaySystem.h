#pragma once

#include "Passenger.h"
#include "Reservation.h"
#include "Train.h"

#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class RailwaySystem {
public:
    RailwaySystem();

    void loadData();
    bool saveData() const;

    void showDashboard() const;
    void addTrain();
    void viewAndSortTrains() const;
    void searchTrains() const;
    void updateTrain();
    void deactivateTrain();

    void addPassenger();
    void viewPassengers() const;
    void searchPassengers() const;

    void bookTicket();
    void cancelTicket();
    void searchPnr() const;
    void viewWaitingList() const;
    void viewStations() const;

private:
    std::vector<Train> trains_;
    std::vector<Passenger> passengers_;
    std::unordered_map<int, std::size_t> trainIndex_;
    std::unordered_map<int, std::size_t> passengerIndex_;
    std::unordered_map<long long, Reservation> reservationsByPnr_;
    std::map<int, std::queue<long long>> waitingLists_;
    std::map<int, std::set<int>> availableSeatNumbers_;
    std::set<std::string> stations_;
    long long nextPnr_;

    void rebuildIndexes();
    void rebuildDerivedData();
    void rebuildSeatsForTrain(int trainNumber);
    void rebuildStations();
    void promoteWaitingPassengers(int trainNumber);
    void removeFromWaitingList(int trainNumber, long long pnr);

    Train* findTrain(int trainNumber);
    const Train* findTrain(int trainNumber) const;
    Passenger* findPassenger(int passengerId);
    const Passenger* findPassenger(int passengerId) const;

    long long generatePnr();
    double calculateFare(const std::string& travelClass) const;
    void displayReservation(const Reservation& reservation) const;
};
