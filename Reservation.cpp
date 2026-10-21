#include "Reservation.h"

#include <iomanip>
#include <iostream>
#include <utility>

Reservation::Reservation(long long pnr, int trainNumber, int passengerId, std::string travelClass,
                         int seatNumber, ReservationStatus status, double fare)
    : pnr_(pnr), trainNumber_(trainNumber), passengerId_(passengerId),
      travelClass_(std::move(travelClass)), seatNumber_(seatNumber), status_(status), fare_(fare) {}

long long Reservation::pnr() const { return pnr_; }
int Reservation::trainNumber() const { return trainNumber_; }
int Reservation::passengerId() const { return passengerId_; }
const std::string& Reservation::travelClass() const { return travelClass_; }
int Reservation::seatNumber() const { return seatNumber_; }
ReservationStatus Reservation::status() const { return status_; }
double Reservation::fare() const { return fare_; }

void Reservation::confirm(int seatNumber) {
    seatNumber_ = seatNumber;
    status_ = ReservationStatus::Confirmed;
}

void Reservation::cancel() { status_ = ReservationStatus::Cancelled; }

void Reservation::display() const {
    std::cout << "PNR          : " << pnr_ << '\n'
              << "Train Number : " << trainNumber_ << '\n'
              << "Passenger ID : " << passengerId_ << '\n'
              << "Class        : " << travelClass_ << '\n'
              << "Seat Number  : " << (seatNumber_ > 0 ? std::to_string(seatNumber_) : "Not assigned") << '\n'
              << "Status       : " << toString(status_) << '\n'
              << std::fixed << std::setprecision(2) << "Fare         : Rs. " << fare_ << '\n';
}

std::string toString(ReservationStatus status) {
    switch (status) {
        case ReservationStatus::Confirmed: return "CONFIRMED";
        case ReservationStatus::Waiting: return "WAITING";
        case ReservationStatus::Cancelled: return "CANCELLED";
    }
    return "UNKNOWN";
}

bool statusFromString(const std::string& text, ReservationStatus& status) {
    if (text == "CONFIRMED") { status = ReservationStatus::Confirmed; return true; }
    if (text == "WAITING") { status = ReservationStatus::Waiting; return true; }
    if (text == "CANCELLED") { status = ReservationStatus::Cancelled; return true; }
    return false;
}
