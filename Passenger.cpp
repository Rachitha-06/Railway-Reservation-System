#include "Passenger.h"

#include <iostream>
#include <utility>

Passenger::Passenger(int id, std::string name, int age, std::string gender, std::string phone)
    : id_(id), name_(std::move(name)), age_(age), gender_(std::move(gender)),
      phone_(std::move(phone)) {}

int Passenger::id() const { return id_; }
const std::string& Passenger::name() const { return name_; }
int Passenger::age() const { return age_; }
const std::string& Passenger::gender() const { return gender_; }
const std::string& Passenger::phone() const { return phone_; }

void Passenger::display() const {
    std::cout << "Passenger ID : " << id_ << '\n'
              << "Name         : " << name_ << '\n'
              << "Age          : " << age_ << '\n'
              << "Gender       : " << gender_ << '\n'
              << "Phone        : " << phone_ << '\n';
}
