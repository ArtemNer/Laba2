#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <clocale>
#ifdef _WIN32
#include <windows.h>
#ifdef max
#undef max
#endif
#endif

using namespace std;

// ---------- Вспомогательные функции ввода ----------
void clearStdin() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string inputNonEmptyString(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        getline(cin, s);
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos)
            s = s.substr(start, end - start + 1);
        else
            s = "";

        if (!s.empty()) return s;
        cout << "Ошибка: поле не может быть пустым. Попробуйте снова.\n";
    }
}

double inputPositiveDouble(const string& prompt) {
    double x;
    while (true) {
        cout << prompt;
        if (!(cin >> x)) {
            cout << "Ошибка: введите число.\n";
            cin.clear();
            clearStdin();
            continue;
        }
        clearStdin();
        if (x > 0.0) return x;
        cout << "Ошибка: значение должно быть > 0. Попробуйте снова.\n";
    }
}

int inputPositiveIntLimited(const string& prompt, int maxAllowed = 10000) {
    string line;
    while (true) {
        cout << prompt;
        if (!std::getline(cin, line)) {
            cout << "Ошибка ввода. Попробуйте снова.\n";
            cin.clear();
            continue;
        }
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end = line.find_last_not_of(" \t\r\n");
        if (start == string::npos || end == string::npos) {
            cout << "Ошибка: поле не может быть пустым. Попробуйте снова.\n";
            continue;
        }
        string s = line.substr(start, end - start + 1);

        bool allDigits = !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { return isdigit(c); });
        if (!allDigits) {
            cout << "Ошибка: введите целое число.\n";
            continue;
        }

        try {
            long long val = stoll(s);
            if (val <= 0) {
                cout << "Ошибка: значение должно быть > 0. Попробуйте снова.\n";
                continue;
            }
            if (val > maxAllowed) {
                cout << "Ошибка: значение не должно превышать " << maxAllowed << ". Попробуйте снова.\n";
                continue;
            }
            return static_cast<int>(val);
        }
        catch (...) {
            cout << "Ошибка: недопустимое число. Попробуйте снова.\n";
            continue;
        }
    }
}

int inputIntInRange(const string& prompt, int low, int high)
{
    setlocale(LC_ALL, "rus");
    int v;
    while (true) {
        cout << prompt;
        if (!(cin >> v)) {
            cout << "Ошибка: введите целое число.\n";
            cin.clear();
            clearStdin();
            continue;
        }
        clearStdin();
        if (v >= low && v <= high) return v;
        cout << "Ошибка: введите число в диапазоне [" << low << ", " << high << "].\n";
    }
}

int inputMenuChoice(const string& prompt, int low, int high) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            cin.clear();
            continue;
        }
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end = line.find_last_not_of(" \t\r\n");
        if (start == string::npos || end == string::npos) {
            cout << "Ошибка: поле не может быть пустым.\n";
            continue;
        }
        string s = line.substr(start, end - start + 1);

        bool allDigits = !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { return isdigit(c); });
        if (!allDigits) {
            cout << "Ошибка: введите целое число\n";
            continue;
        }


        int val = stoi(s);
        if (val < low || val > high) {
            cout << "Ошибка: число вне допустимого диапазона [" << low << ", " << high << "].\n";
            continue;
        }
        return val;
    }
}

// ---------- Сущности предметной области ----------

class Room {
public:
    int id;
    string number; // номер комнаты (например "101" или "A-12")
    double costPerNight;
    bool occupied;
    int occupantClientId; // -1 если свободна

    Room(int id_, const string& number_, double cost_)
        : id(id_), number(number_), costPerNight(cost_), occupied(false), occupantClientId(-1) {
    }

    void printBrief() const {
        cout << "ID: " << id
            << " | Номер: " << number
            << " | Стоимость: " << fixed << setprecision(2) << costPerNight
            << " | " << (occupied ? "Занят" : "Свободен");
        if (occupied) cout << " (ClientID=" << occupantClientId << ")";
        cout << "\n";
    }
};

class Client {
private:
    int id;
    string surname;
    int roomId; // id забронированной комнаты или -1

public:
    Client(int id_, const string& surname_)
        : id(id_), surname(surname_), roomId(-1) {
    }

    int getId() const { return id; }
    string getSurname() const { return surname; }
    int getRoomId() const { return roomId; }

    void assignRoom(int rId) { roomId = rId; }
    void clearRoom() { roomId = -1; }

    void printBrief() const {
        cout << "ID: " << id << " | Фамилия: " << surname
            << " | Номер комнаты (ID): " << (roomId == -1 ? string("нет") : to_string(roomId)) << "\n";
    }
};

// ---------- Система гостиницы (Singleton) ----------

class HotelSystem {
private:
    HotelSystem() : nextRoomId(1), nextClientId(1) {}

    vector<unique_ptr<Room>> rooms;
    vector<unique_ptr<Client>> clients;

    int nextRoomId;
    int nextClientId;

public:
    // запрет копирования/перемещения
    HotelSystem(const HotelSystem&) = delete;
    HotelSystem& operator=(const HotelSystem&) = delete;
    HotelSystem(HotelSystem&&) = delete;
    HotelSystem& operator=(HotelSystem&&) = delete;

    static HotelSystem& getInstance() {
        static HotelSystem instance;
        return instance;
    }

    // Комнаты
    int addRoom(const string& number, double cost) {
        int id = nextRoomId++;
        rooms.push_back(make_unique<Room>(id, number, cost));
        return id;
    }

    const vector<unique_ptr<Room>>& getRooms() const { return rooms; }

    Room* findRoomById(int id) {
        for (auto& r : rooms) if (r->id == id) return r.get();
        return nullptr;
    }

    // Клиенты
    int addClient(const string& surname) {
        int id = nextClientId++;
        clients.push_back(make_unique<Client>(id, surname));
        return id;
    }

    Client* findClientById(int id) {
        for (auto& c : clients) if (c->getId() == id) return c.get();
        return nullptr;
    }

    Client* findClientBySurname(const string& surname) {
        for (auto& c : clients) if (c->getSurname() == surname) return c.get();
        return nullptr;
    }

    const vector<unique_ptr<Client>>& getClients() const { return clients; }

    // Бронирование: назначить клиенту комнату
    // возврат: 0 - успех, 1 - клиент не найден, 2 - комната не найдена, 3 - уже занята, 4 - клиент уже имеет комнату
    int bookRoom(int clientId, int roomId) {
        Client* cl = findClientById(clientId);
        if (!cl) return 1;
        Room* rm = findRoomById(roomId);
        if (!rm) return 2;
        if (rm->occupied) return 3;
        if (cl->getRoomId() != -1) return 4;

        // назначаем
        rm->occupied = true;
        rm->occupantClientId = clientId;
        cl->assignRoom(roomId);
        return 0;
    }


    // Снять бронь (необязательно, но можно)
    bool cancelBooking(int clientId) {
        Client* cl = findClientById(clientId);
        if (!cl) return false;
        int rid = cl->getRoomId();
        if (rid == -1) return false;
        Room* rm = findRoomById(rid);
        if (!rm) return false;
        rm->occupied = false;
        rm->occupantClientId = -1;
        cl->clearRoom();
        return true;
    }

    vector<Room*> getAvailableRooms() {
        vector<Room*> res;
        for (auto& r : rooms) if (!r->occupied) res.push_back(r.get());
        return res;
    }

    // после ввода фамилии клиента вывод стоимости проживания (если забронил) — возвращает true/false
    bool getCostByClientSurname(const string& surname, double& outCost) {
        Client* cl = findClientBySurname(surname);
        if (!cl) return false;
        int rid = cl->getRoomId();
        if (rid == -1) {
            outCost = 0.0;
            return true; // клиент найден, но не забронировал — caller должен понять различие (roomId==-1)
        }
        Room* rm = findRoomById(rid);
        if (!rm) return false;
        outCost = rm->costPerNight;
        return true;
    }
};

// ---------- Команды меню ----------

void printMenu() {
    cout << "\n=== Меню информационной системы гостиницы ===\n";
    cout << "1. Ввести информацию о номере (номер + стоимость)\n";
    cout << "2. Показать все номера\n";
    cout << "3. Зарегистрировать клиента и заказать номер\n";
    cout << "4. Показать список свободных номеров\n";
    cout << "5. Ввести фамилию клиента -> показать стоимость проживания\n";
    cout << "6. Показать всех клиентов\n";
    cout << "0. Выход\n";
    cout << "============================================\n";
}

void cmdAddRoom() {
    string number = inputNonEmptyString("Введите обозначение номера (например 101, A-12): ");
    double cost;
    while (true) {
        cost = inputPositiveDouble("Введите стоимость проживания за ночь: ");
        if (cost <= 1000000.0) break;
        cout << "Ошибка: стоимость слишком велика. Попробуйте снова.\n";
    }
    int id = HotelSystem::getInstance().addRoom(number, cost);
    cout << "Комната добавлена. ID = " << id << "\n";
}

void cmdListRooms() {
    const auto& rooms = HotelSystem::getInstance().getRooms();
    if (rooms.empty()) {
        cout << "Список комнат пуст.\n";
        return;
    }
    cout << "Список комнат:\n";
    for (const auto& r : rooms) {
        r->printBrief();
    }
}

void cmdRegisterClientAndBook() {
    string surname = inputNonEmptyString("Введите фамилию клиента: ");
    int clientId = HotelSystem::getInstance().addClient(surname);
    cout << "Клиент зарегистрирован. ID = " << clientId << "\n";

    // показать доступные номера
    auto available = HotelSystem::getInstance().getAvailableRooms();
    if (available.empty()) {
        cout << "Нет свободных номеров для бронирования.\n";
        return;
    }

    cout << "Свободные номера:\n";
    for (const auto& r : available) {
        cout << "ID: " << r->id << " | Номер: " << r->number
            << " | Стоимость: " << fixed << setprecision(2) << r->costPerNight << "\n";
    }

    int roomId = inputIntInRange("Введите ID номера для бронирования (или 0 чтобы отменить): ", 0, 1000000);
    if (roomId == 0) {
        cout << "Бронирование отменено пользователем.\n";
        return;
    }


    int res = HotelSystem::getInstance().bookRoom(clientId, roomId);
    if (res == 0) {
        Room* rm = HotelSystem::getInstance().findRoomById(roomId);
        cout << "Номер " << rm->number << " успешно забронирован за клиентом " << surname << ".\n";
    }
    else if (res == 1) {
        cout << "Ошибка: клиент не найден.\n";
    }
    else if (res == 2) {
        cout << "Ошибка: номер с таким ID не найден.\n";
    }
    else if (res == 3) {
        cout << "Внимание: выбранный номер уже занят. Бронирование не выполнено.\n";
    }
    else if (res == 4) {
        cout << "Внимание: у клиента уже есть забронированный номер.\n";
    }
    else {
        cout << "Неизвестная ошибка при бронировании.\n";
    }
}

void cmdListAvailableRooms() {
    auto avail = HotelSystem::getInstance().getAvailableRooms();
    if (avail.empty()) {
        cout << "Свободных номеров нет.\n";
        return;
    }
    cout << "Свободные номера:\n";
    for (const auto& r : avail) {
        cout << "ID: " << r->id << " | Номер: " << r->number
            << " | Стоимость: " << fixed << setprecision(2) << r->costPerNight << "\n";
    }
}

void cmdCostByClientSurname() {
    string surname = inputNonEmptyString("Введите фамилию клиента: ");
    double cost;
    bool ok = HotelSystem::getInstance().getCostByClientSurname(surname, cost);
    if (!ok) {
        cout << "Клиент с фамилией '" << surname << "' не найден.\n";
        return;
    }
    Client* cl = HotelSystem::getInstance().findClientBySurname(surname);
    if (cl->getRoomId() == -1) {
        cout << "Клиент найден, но номер не забронирован. Стоимость проживания отсутствует.\n";
    }
    else {
        cout << "Клиент " << surname << " оплатит/оплачивает за проживание: " << fixed << setprecision(2) << cost << " (за ночь).\n";
    }
}

void cmdListClients() {
    const auto& clients = HotelSystem::getInstance().getClients();
    if (clients.empty()) {
        cout << "Список клиентов пуст.\n";
        return;
    }
    cout << "Список клиентов:\n";
    for (const auto& c : clients) {
        c->printBrief();
    }
}

// ---------- main ----------
int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif
    setlocale(LC_ALL, "Russian");

    bool running = true;
    while (running) {
        printMenu();
        int choice = inputMenuChoice("Выберите пункт меню: ", 0, 6);
        switch (choice) {
        case 1: cmdAddRoom(); break;
        case 2: cmdListRooms(); break;
        case 3: cmdRegisterClientAndBook(); break;
        case 4: cmdListAvailableRooms(); break;
        case 5: cmdCostByClientSurname(); break;
        case 6: cmdListClients(); break;
        case 0:
            running = false;
            cout << "Выход из программы.\n";
            break;
        default:
            cout << "Неверный выбор.\n";
            break;
        }
    }

    cout << "Программа завершена.\n";
    return 0;
}
