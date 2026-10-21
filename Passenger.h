#pragma once

#include <string>

class Passenger {
public:
    Passenger(int id, std::string name, int age, std::string gender, std::string phone);

    int id() const;
    const std::string& name() const;
    int age() const;
    const std::string& gender() const;
    const std::string& phone() const;
    void display() const;

private:
    int id_;
    std::string name_;
    int age_;
    std::string gender_;
    std::string phone_;
};
