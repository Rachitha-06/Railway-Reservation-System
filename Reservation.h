#pragma once

#include <string>

enum class ReservationStatus { Confirmed, Waiting, Cancelled };

class Reservation {
public:
    Reservation(long long pnr, int trainNumber, int passengerId, std::string travelClass,
                int seatNumber, ReservationStatus status, double fare);

    long long pnr() const;
    int trainNumber() const;
    int passengerId() const;
    const std::string& travelClass() const;
    int seatNumber() const;
    ReservationStatus status() const;
    double fare() const;

    void confirm(int seatNumber);
    void cancel();
    void display() const;

private:
    long long pnr_;
    int trainNumber_;
    int passengerId_;
    std::string travelClass_;
    int seatNumber_;
    ReservationStatus status_;
    double fare_;
};

std::string toString(ReservationStatus status);
bool statusFromString(const std::string& text, ReservationStatus& status);
