#include "RailwaySystem.h"

#include <iostream>
#include <limits>

namespace {
int readMenuChoice() {
    while (true) {
        std::cout << "Choose an option: ";
        int choice = 0;
        if (std::cin >> choice) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
        std::cout << "Enter a whole-number menu option.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void showMenu() {
    std::cout << "\n========================================\n"
              << "                RAILFLOW\n"
              << "========================================\n"
              << " 1. Railway dashboard\n"
              << " 2. Add train\n"
              << " 3. View / sort trains\n"
              << " 4. Search trains\n"
              << " 5. Update train\n"
              << " 6. Deactivate train\n"
              << " 7. Add passenger\n"
              << " 8. View passengers\n"
              << " 9. Search passengers\n"
              << "10. Book ticket\n"
              << "11. Cancel ticket\n"
              << "12. Search PNR / view ticket\n"
              << "13. View waiting list\n"
              << "14. View stations\n"
              << "15. Save and exit\n";
}
}  // namespace

int main() {
    RailwaySystem railway;
    railway.loadData();

    while (true) {
        showMenu();
        switch (readMenuChoice()) {
            case 1: railway.showDashboard(); break;
            case 2: railway.addTrain(); break;
            case 3: railway.viewAndSortTrains(); break;
            case 4: railway.searchTrains(); break;
            case 5: railway.updateTrain(); break;
            case 6: railway.deactivateTrain(); break;
            case 7: railway.addPassenger(); break;
            case 8: railway.viewPassengers(); break;
            case 9: railway.searchPassengers(); break;
            case 10: railway.bookTicket(); break;
            case 11: railway.cancelTicket(); break;
            case 12: railway.searchPnr(); break;
            case 13: railway.viewWaitingList(); break;
            case 14: railway.viewStations(); break;
            case 15:
                if (railway.saveData()) std::cout << "Data saved. Goodbye.\n";
                else std::cout << "Data could not be saved. Existing files were left open for inspection.\n";
                return 0;
            default: std::cout << "Invalid menu choice.\n";
        }
    }
}
