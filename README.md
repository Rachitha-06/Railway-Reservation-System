# RailFlow

RailFlow is a C++17 console railway-reservation simulator designed to demonstrate OOP, STL, file handling, searching, sorting, validation, and modular programming without a database or external libraries.

## Build and run

From the `RailFlow` folder:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp Train.cpp Passenger.cpp Reservation.cpp RailwaySystem.cpp -o railflow.exe
.\railflow.exe
```

The included `data` folder receives `trains.txt`, `passengers.txt`, and `reservations.txt` when you choose **Save and exit**. The application reloads them on the next start.

## Features

- Add, search, sort, update, view, and deactivate trains.
- Add, view, and search passengers.
- Book confirmed tickets and generate unique PNR values beginning at `500001`.
- Allocate the lowest free seat number; release it on cancellation.
- Add a booking to a per-train waiting list when a train is full.
- Promote the earliest waiting reservation when a confirmed seat is cancelled.
- Search a ticket by PNR and view the railway dashboard.
- Validate menu options, numeric fields, duplicate IDs, missing records, invalid routes, inactive trains, and file records.

## STL choices

| Container / algorithm | Purpose | Why it fits |
|---|---|---|
| `vector<Train>`, `vector<Passenger>` | Primary ordered collections | Easy iteration for displaying and searching; grows dynamically. |
| `unordered_map<int, size_t>` | Train/passenger ID to vector index | Average O(1) lookup while retaining the vectors for ordered views. |
| `unordered_map<long long, Reservation>` | PNR to reservation | Average O(1) PNR search and cancellation. |
| `map<int, queue<long long>>` | Per-train waiting lists | Sorted train keys and FIFO queue order. |
| `set<int>` | Free seats per train | Guarantees unique seats and makes the lowest seat easy to assign. |
| `set<string>` | Station names | Keeps each station unique and alphabetically ordered. |
| `find_if`, `count_if`, `sort`, `transform`, `accumulate` | Search, dashboard, sort, normalization | Standard tested algorithms express intent clearly. |

## Architecture

- `Train`: train information and active/capacity state.
- `Passenger`: passenger profile.
- `Reservation`: PNR, class, seat, fare, and an `enum class` status (`CONFIRMED`, `WAITING`, `CANCELLED`).
- `RailwaySystem`: application rules, STL containers, persistence, and menu operations.
- `main.cpp`: only program startup and menu routing.

`Station` intentionally is not a class. A `set<string>` is enough because stations have no independent attributes in this simulator. A separate `FileManager` is also unnecessary at this scale; `RailwaySystem` owns the data it persists.

## File format

Records use `|` as a delimiter, so interactive text fields reject that character. The program writes complete collections using `ofstream` and recreates objects line by line using `ifstream` at startup. Unreadable files and malformed records are reported; malformed records are skipped safely.

## Example booking flow

1. Add a train with three seats.
2. Add four passengers.
3. Book four tickets on that train. The first three are confirmed; accept the waiting-list prompt for the fourth.
4. Cancel the first confirmed PNR. The first waiting PNR is promoted and receives the released seat.
5. Save, restart, and search the promoted PNR to confirm persistence.

## Known limitations and next improvements

This is intentionally an educational simulation: fares are fixed by class, time text is not validated as an actual clock time, each train has one pooled seat inventory rather than class-specific coaches, and files are plain text rather than crash-atomic storage. Natural next steps would be date-aware journeys, stronger time/phone validation, per-class capacities, CSV escaping, and atomic temporary-file saves.

## Suggested Git milestone

```text
git add .
git commit -m "feat: complete RailFlow reservation and persistence system"
```

See `PROJECT_REVIEW_AND_INTERVIEW.md` for the final implementation review, manual acceptance tests, and interview questions tied to this code.
