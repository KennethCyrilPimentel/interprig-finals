#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm> // For std::transform, std::find, std::remove_if, std::find_if
#include <limits>    // For std::numeric_limits
#include <map>       // For inventory allocation in events
#include <locale>    // For std::locale

// Forward declarations
class User;
class Admin;
class RegularUser;
class Event;
class Attendee;
class InventoryItem;
class System;

// --- Enums ---
enum class Role { ADMIN, REGULAR_USER, NONE };
enum class EventStatus { UPCOMING, ONGOING, COMPLETED, CANCELED };

// --- Helper Functions ---

// Function to convert string to lowercase
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// Function to get validated string input (ensures not empty)
std::string getStringInput(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        if (!input.empty()) {
            return input;
        }
        std::cout << "Input cannot be empty. Please try again.\n";
    }
}

// Function to get validated integer input
int getIntInput(const std::string& prompt) {
    int input;
    while (true) {
        std::cout << prompt;
        std::cin >> input;
        if (std::cin.good()) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
            return input;
        }
        std::cout << "Invalid input. Please enter an integer.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// Function to get validated positive integer input
int getPositiveIntInput(const std::string& prompt) {
    int input;
    while (true) {
        input = getIntInput(prompt);
        if (input > 0) {
            return input;
        }
        std::cout << "Input must be a positive integer. Please try again.\n";
    }
}

// Basic date validation (format-MM-DD)
bool isValidDate(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

// Basic time validation (format HH:MM - 24-hour)
bool isValidTime(const std::string& time) {
    if (time.length() != 5) return false;
    if (time[2] != ':') return false;
     try {
        int hour = std::stoi(time.substr(0, 2));
        int minute = std::stoi(time.substr(3, 2));
        if (hour < 0 || hour > 23) return false;
        if (minute < 0 || minute > 59) return false;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}


// --- Class Definitions ---

// ** User Class (Abstract Base Class) **
class User {
protected:
    std::string username;
    std::string password;
    Role role;
    int userId;
    static int nextUserId;

public:
    User(std::string uname, std::string pwd, Role r);
    User(int id, std::string uname, std::string pwd, Role r);
    virtual ~User();

    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }
    Role getRole() const { return role; }
    int getUserId() const { return userId; }

    void setPassword(const std::string& newPassword);

    virtual void displayMenu(System& sys) = 0; // Pure virtual
    virtual std::string toString() const;
    static User* fromString(const std::string& str); // Definition after Admin/RegularUser
    static void initNextId(int id) { if (id >= nextUserId) nextUserId = id + 1;}
};
int User::nextUserId = 1;


// ** Admin Class **
class Admin : public User {
public:
    Admin(std::string uname, std::string pwd);
    Admin(int id, std::string uname, std::string pwd);
    void displayMenu(System& sys) override; // Definition moved out
private:
    void adminUserManagementMenu(System& sys);
    void adminEventManagementMenu(System& sys);
    void adminAttendeeManagementMenu(System& sys);
    void adminInventoryManagementMenu(System& sys);
    void adminDataExportMenu(System& sys);
};

// ** RegularUser Class **
class RegularUser : public User {
public:
    RegularUser(std::string uname, std::string pwd);
    RegularUser(int id, std::string uname, std::string pwd);
    void displayMenu(System& sys) override; // Definition moved out
};


// ** Attendee Class **
class Attendee {
public:
    int attendeeId;
    std::string name;
    std::string contactInfo;
    int eventIdRegisteredFor;
    bool isCheckedIn;
    static int nextAttendeeId;

    Attendee(std::string n, std::string contact, int eventId);
    Attendee(int id, std::string n, std::string contact, int eventId, bool checkedInStatus);
    void checkIn();
    void displayDetails() const;
    std::string toString() const;
    static Attendee fromString(const std::string& str);
    static void initNextId(int id) { if (id >= nextAttendeeId) nextAttendeeId = id + 1;}
};
int Attendee::nextAttendeeId = 1;

// ** InventoryItem Class **
class InventoryItem {
public:
    int itemId;
    std::string name;
    int totalQuantity;
    int allocatedQuantity;
    std::string description;
    static int nextItemId;

    InventoryItem(std::string n, int qty, std::string desc);
    InventoryItem(int id, std::string n, int totalQty, int allocQty, std::string desc);
    int getAvailableQuantity() const;
    bool allocate(int quantityToAllocate);
    bool deallocate(int quantityToDeallocate);
    void setTotalQuantity(int newTotalQuantity);
    void displayDetails() const;
    std::string toString() const;
    static InventoryItem fromString(const std::string& str);
    static void initNextId(int id) { if (id >= nextItemId) nextItemId = id + 1;}
};
int InventoryItem::nextItemId = 1;

// ** Event Class **
class Event {
public:
    int eventId;
    std::string name;
    std::string date;
    std::string time;
    std::string location;
    std::string description;
    std::string category;
    EventStatus status;
    std::vector<int> attendeeIds;
    std::map<int, int> allocatedInventory;
    static int nextEventId;

    Event(std::string n, std::string d, std::string t, std::string loc, std::string desc, std::string cat);
    Event(int id, std::string n, std::string d, std::string t, std::string loc,
          std::string desc, std::string cat, EventStatus stat);
    void addAttendee(int attendeeId);
    void removeAttendee(int attendeeId);
    void allocateInventoryItem(int itemId, int quantity);
    int deallocateInventoryItem(int itemId, int quantityToDeallocate);
    std::string getStatusString() const;
    void displayDetails(const System& sys) const; // Definition after System
    std::string attendeesToString() const;
    std::string inventoryToString() const;
    std::string toString() const;
    static Event fromString(const std::string& str);
    static void initNextId(int id) { if (id >= nextEventId) nextEventId = id + 1;}
};
int Event::nextEventId = 1;

// ** System Class **
class System {
private:
    void seedInitialData(); // DECLARATION - Definition moved out

public:
    std::vector<User*> users;
    std::vector<Event> events;
    std::vector<InventoryItem> inventory;
    std::vector<Attendee> allAttendees;
    User* currentUser;

    const std::string USERS_FILE = "users.txt";
    const std::string EVENTS_FILE = "events.txt";
    const std::string INVENTORY_FILE = "inventory.txt";
    const std::string ATTENDEES_FILE = "attendees.txt";

    System() : currentUser(nullptr) {}
    ~System();

    void loadData();
    void saveData();
    void loadUsers(); // Definition after Admin/RegularUser & User::fromString
    void saveUsers();
    void loadEvents();
    void saveEvents();
    void loadInventory();
    void saveInventory();
    void loadAttendees();
    void saveAttendees();

    bool usernameExists(const std::string& username) const;
    void createUserAccount(const std::string& uname, const std::string& pwd, Role role); // Definition after Admin/RegularUser
    void publicRegisterNewUser(); // Definition after Admin/RegularUser
    void deleteUserAccount(const std::string& uname);
    User* findUserByUsername(const std::string& uname);
    const User* findUserByUsername(const std::string& uname) const;
    void listAllUsers() const;

    bool login();
    void logout();

    Event* findEventById(int eventId);
    const Event* findEventById(int eventId) const;
    void createEvent();
    void viewAllEvents(bool adminView = false) const;
    void searchEventsByNameOrDate() const;
    void editEventDetails();
    void deleteEvent();
    void updateEventStatus();

    Attendee* findAttendeeInMasterList(int attendeeId);
    const Attendee* findAttendeeInMasterList(int attendeeId) const;
    void registerAttendeeForEvent();
    void cancelOwnRegistration();
    void viewAttendeeListsPerEvent() const;
    void checkInAttendeeForEvent();
    void generateAttendanceReportForEvent() const;
    void exportAttendeeListForEventToFile() const;

    InventoryItem* findInventoryItemById(int itemId);
    const InventoryItem* findInventoryItemById(int itemId) const;
    InventoryItem* findInventoryItemByName(const std::string& name);
    const InventoryItem* findInventoryItemByName(const std::string& name) const;
    void addInventoryItem();
    void updateInventoryItemDetails();
    void viewAllInventoryItems() const;
    void trackInventoryAllocationToEvent();
    void generateFullInventoryReport() const;

    void exportAllEventsDataToFile() const;
    void exportAllAttendeesDataToFile() const;
    void exportAllInventoryDataToFile() const;

    void run(); // Definition after Admin/RegularUser displayMenu
    void updateCurrentLoggedInUserContactInfo();
};

// --- User Class Method Definitions ---
User::User(std::string uname, std::string pwd, Role r)
    : username(std::move(uname)), password(std::move(pwd)), role(r) {
    userId = nextUserId++;
}
User::User(int id, std::string uname, std::string pwd, Role r)
    : userId(id), username(std::move(uname)), password(std::move(pwd)), role(r) {
    if (id >= nextUserId) {
        nextUserId = id + 1;
    }
}
User::~User() {}

void User::setPassword(const std::string& newPassword) {
    if (newPassword.length() < 6) {
        std::cout << "Password must be at least 6 characters long.\n";
        return;
    }
    password = newPassword;
    std::cout << "Password updated successfully.\n";
}

std::string User::toString() const {
    std::stringstream ss;
    ss << userId << "," << username << "," << password << "," << static_cast<int>(role);
    return ss.str();
}

// --- Admin Class Method Definitions ---
Admin::Admin(std::string uname, std::string pwd) : User(std::move(uname), std::move(pwd), Role::ADMIN) {}
Admin::Admin(int id, std::string uname, std::string pwd) : User(id, std::move(uname), std::move(pwd), Role::ADMIN) {}

// --- RegularUser Class Method Definitions ---
RegularUser::RegularUser(std::string uname, std::string pwd) : User(std::move(uname), std::move(pwd), Role::REGULAR_USER) {}
RegularUser::RegularUser(int id, std::string uname, std::string pwd) : User(id, std::move(uname), std::move(pwd), Role::REGULAR_USER) {}


// --- User Factory Method Definition (User::fromString) ---
User* User::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string segment;
    int id;
    std::string uname, pwd;
    Role role_val;

    if (str.empty() || std::count(str.begin(), str.end(), ',') < 3) {
        std::cerr << "Warning: Malformed user data line: '" << str << "'. Skipping.\n";
        return nullptr;
    }
    try {
        std::getline(ss, segment, ','); id = std::stoi(segment);
        std::getline(ss, uname, ',');
        std::getline(ss, pwd, ',');
        std::getline(ss, segment, ','); role_val = static_cast<Role>(std::stoi(segment));
    } catch (const std::exception& e) {
        std::cerr << "Warning: Invalid data format in user line '" << str << "': " << e.what() << ". Skipping.\n";
        return nullptr;
    }

    if (role_val == Role::ADMIN) {
        return new Admin(id, uname, pwd);
    } else if (role_val == Role::REGULAR_USER) {
        return new RegularUser(id, uname, pwd);
    }
    std::cerr << "Warning: Unknown role in user data line: '" << str << "'. Skipping.\n";
    return nullptr;
}

// --- Attendee Class Method Definitions ---
Attendee::Attendee(std::string n, std::string contact, int eventId)
    : name(std::move(n)), contactInfo(std::move(contact)), eventIdRegisteredFor(eventId), isCheckedIn(false) {
    attendeeId = nextAttendeeId++;
}
Attendee::Attendee(int id, std::string n, std::string contact, int eventId, bool checkedInStatus)
    : attendeeId(id), name(std::move(n)), contactInfo(std::move(contact)),
      eventIdRegisteredFor(eventId), isCheckedIn(checkedInStatus) {
    if (id >= nextAttendeeId) {
        nextAttendeeId = id + 1;
    }
}
void Attendee::checkIn() {
    if (!isCheckedIn) {
        isCheckedIn = true;
        std::cout << name << " checked in successfully for event ID " << eventIdRegisteredFor << ".\n";
    } else {
        std::cout << name << " is already checked in for event ID " << eventIdRegisteredFor << ".\n";
    }
}
void Attendee::displayDetails() const {
    std::cout << "Attendee ID: " << attendeeId
              << ", Name: " << name
              << ", Contact: " << contactInfo
              << ", Registered for Event ID: " << (eventIdRegisteredFor == 0 ? "N/A (Profile)" : std::to_string(eventIdRegisteredFor))
              << ", Checked-in: " << (isCheckedIn ? "Yes" : "No") << std::endl;
}
std::string Attendee::toString() const {
    std::stringstream ss;
    ss << attendeeId << "," << name << "," << contactInfo << "," << eventIdRegisteredFor << "," << (isCheckedIn ? "1" : "0");
    return ss.str();
}
Attendee Attendee::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string segment;
    int id, eventId;
    std::string name, contact;
    bool checkedIn;
    std::getline(ss, segment, ','); id = std::stoi(segment);
    std::getline(ss, name, ',');
    std::getline(ss, contact, ',');
    std::getline(ss, segment, ','); eventId = std::stoi(segment);
    std::getline(ss, segment, ','); checkedIn = (segment == "1");
    return Attendee(id, name, contact, eventId, checkedIn);
}

// --- InventoryItem Class Method Definitions ---
InventoryItem::InventoryItem(std::string n, int qty, std::string desc)
    : name(std::move(n)), totalQuantity(qty), allocatedQuantity(0), description(std::move(desc)) {
    itemId = nextItemId++;
}
InventoryItem::InventoryItem(int id, std::string n, int totalQty, int allocQty, std::string desc)
    : itemId(id), name(std::move(n)), totalQuantity(totalQty), allocatedQuantity(allocQty), description(std::move(desc)) {
    if (id >= nextItemId) {
        nextItemId = id + 1;
    }
}
int InventoryItem::getAvailableQuantity() const { return totalQuantity - allocatedQuantity; }
bool InventoryItem::allocate(int quantityToAllocate) {
    if (quantityToAllocate <= 0) { std::cout << "Error: Allocation quantity must be positive.\n"; return false; }
    if (quantityToAllocate <= getAvailableQuantity()) {
        allocatedQuantity += quantityToAllocate;
        return true;
    }
    std::cout << "Error: Not enough '" << name << "' available. Available: " << getAvailableQuantity() << std::endl;
    return false;
}
bool InventoryItem::deallocate(int quantityToDeallocate) {
    if (quantityToDeallocate <= 0) { std::cout << "Error: Deallocation quantity must be positive.\n"; return false; }
    if (quantityToDeallocate <= allocatedQuantity) {
        allocatedQuantity -= quantityToDeallocate;
        return true;
    }
    std::cout << "Error: Cannot deallocate " << quantityToDeallocate << " of '" << name << "'. Allocated: " << allocatedQuantity << std::endl;
    return false;
}
void InventoryItem::setTotalQuantity(int newTotalQuantity) {
    if (newTotalQuantity < 0) { std::cout << "Error: Total quantity cannot be negative.\n"; return; }
    if (newTotalQuantity < allocatedQuantity) {
        std::cout << "Error: New total quantity (" << newTotalQuantity << ") cannot be less than allocated (" << allocatedQuantity << ").\n";
        return;
    }
    totalQuantity = newTotalQuantity;
    std::cout << "Total quantity for '" << name << "' updated to " << totalQuantity << ".\n";
}
void InventoryItem::displayDetails() const {
    std::cout << "Item ID: " << itemId << ", Name: " << name
              << ", Total: " << totalQuantity
              << ", Allocated: " << allocatedQuantity
              << ", Available: " << getAvailableQuantity()
              << ", Desc: " << description << std::endl;
}
std::string InventoryItem::toString() const {
    std::stringstream ss;
    ss << itemId << "," << name << "," << totalQuantity << "," << allocatedQuantity << "," << description;
    return ss.str();
}
InventoryItem InventoryItem::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string segment, name, desc;
    int id, totalQty, allocQty;
    std::getline(ss, segment, ','); id = std::stoi(segment);
    std::getline(ss, name, ',');
    std::getline(ss, segment, ','); totalQty = std::stoi(segment);
    std::getline(ss, segment, ','); allocQty = std::stoi(segment);
    std::getline(ss, desc);
    return InventoryItem(id, name, totalQty, allocQty, desc);
}

// --- Event Class Method Definitions ---
Event::Event(std::string n, std::string d, std::string t, std::string loc, std::string desc, std::string cat)
    : name(std::move(n)), date(std::move(d)), time(std::move(t)), location(std::move(loc)),
      description(std::move(desc)), category(std::move(cat)), status(EventStatus::UPCOMING) {
    eventId = nextEventId++;
}
Event::Event(int id, std::string n, std::string d, std::string t, std::string loc,
      std::string desc, std::string cat, EventStatus stat)
    : eventId(id), name(std::move(n)), date(std::move(d)), time(std::move(t)), location(std::move(loc)),
      description(std::move(desc)), category(std::move(cat)), status(stat) {
    if (id >= nextEventId) {
        nextEventId = id + 1;
    }
}
void Event::addAttendee(int attId) {
    if (std::find(attendeeIds.begin(), attendeeIds.end(), attId) == attendeeIds.end()) {
        attendeeIds.push_back(attId);
    } else {
        std::cout << "Info: Attendee ID " << attId << " already registered for event '" << name << "'.\n";
    }
}
void Event::removeAttendee(int attId) {
    auto it = std::find(attendeeIds.begin(), attendeeIds.end(), attId);
    if (it != attendeeIds.end()) {
        attendeeIds.erase(it);
    }
}
void Event::allocateInventoryItem(int itmId, int quantity) {
    if (quantity > 0) allocatedInventory[itmId] += quantity;
}
int Event::deallocateInventoryItem(int itmId, int quantityToDeallocate) {
    if (quantityToDeallocate <= 0) return 0;
    auto it = allocatedInventory.find(itmId);
    if (it != allocatedInventory.end()) {
        int currentQty = it->second;
        int actualDeallocated = std::min(currentQty, quantityToDeallocate);
        it->second -= actualDeallocated;
        if (it->second <= 0) {
            allocatedInventory.erase(it);
        }
        return actualDeallocated;
    }
    return 0;
}
std::string Event::getStatusString() const {
    switch (status) {
        case EventStatus::UPCOMING: return "Upcoming";
        case EventStatus::ONGOING: return "Ongoing";
        case EventStatus::COMPLETED: return "Completed";
        case EventStatus::CANCELED: return "Canceled";
        default: return "Unknown";
    }
}
std::string Event::attendeesToString() const {
    std::stringstream ss;
    for (size_t i = 0; i < attendeeIds.size(); ++i) {
        ss << attendeeIds[i] << (i == attendeeIds.size() - 1 ? "" : ";");
    }
    return ss.str();
}
std::string Event::inventoryToString() const {
    std::stringstream ss;
    bool first = true;
    for (auto const& [itemId, quantity] : allocatedInventory) {
        if (!first) ss << ";";
        ss << itemId << ":" << quantity;
        first = false;
    }
    return ss.str();
}
std::string Event::toString() const {
    std::stringstream ss;
    ss << eventId << "," << name << "," << date << "," << time << "," << location << ","
       << description << "," << category << "," << static_cast<int>(status) << ","
       << attendeesToString() << "," << inventoryToString();
    return ss.str();
}
Event Event::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string segment, name, date_str, time_str, loc, desc, cat, attendeesStr, inventoryStr;
    int id;
    EventStatus stat;
    std::getline(ss, segment, ','); id = std::stoi(segment);
    std::getline(ss, name, ',');
    std::getline(ss, date_str, ',');
    std::getline(ss, time_str, ',');
    std::getline(ss, loc, ',');
    std::getline(ss, desc, ',');
    std::getline(ss, cat, ',');
    std::getline(ss, segment, ','); stat = static_cast<EventStatus>(std::stoi(segment));
    if (std::getline(ss, attendeesStr, ',')) {
        std::getline(ss, inventoryStr);
    } else {
        inventoryStr = "";
    }
    Event event(id, name, date_str, time_str, loc, desc, cat, stat);
    if (!attendeesStr.empty()) {
        std::stringstream attSs(attendeesStr);
        std::string attIdStr;
        while (std::getline(attSs, attIdStr, ';')) {
            if(!attIdStr.empty()) event.attendeeIds.push_back(std::stoi(attIdStr));
        }
    }
    if (!inventoryStr.empty()) {
        std::stringstream invSs(inventoryStr);
        std::string itemStr;
        while (std::getline(invSs, itemStr, ';')) {
            if(!itemStr.empty()){
                size_t colonPos = itemStr.find(':');
                if (colonPos != std::string::npos) {
                    try {
                        event.allocatedInventory[std::stoi(itemStr.substr(0, colonPos))] = std::stoi(itemStr.substr(colonPos + 1));
                    } catch (const std::exception& e) { /* ignore malformed */ }
                }
            }
        }
    }
    return event;
}


// --- System Method Definitions ---
System::~System() {
    saveData();
    for (User* u : users) delete u;
    users.clear();
}

void System::seedInitialData() {
    bool dataSeeded = false;
    if (users.empty()) {
        std::cout << "Info: No users found. Seeding initial accounts.\n";
        users.push_back(new Admin("admin", "adminpass"));
        std::cout << "Seeded Admin: admin (ID: " << users.back()->getUserId() << ")\n";
        users.push_back(new RegularUser("user1", "user1pass"));
        std::cout << "Seeded User: user1 (ID: " << users.back()->getUserId() << ")\n";
        users.push_back(new RegularUser("user2", "user2pass"));
        std::cout << "Seeded User: user2 (ID: " << users.back()->getUserId() << ")\n";
        dataSeeded = true;
    }
    if (events.empty()) {
        std::cout << "Info: No events found. Seeding initial events.\n";
        events.emplace_back("Tech Conference 2025", "2025-10-20", "09:00", "Grand Hall", "Annual tech conference", "Conference");
        std::cout << "Seeded Event: Tech Conference 2025 (ID: " << events.back().eventId << ")\n";
        events.emplace_back("Summer Music Festival", "2025-07-15", "14:00", "City Park", "Outdoor music event", "Social");
        std::cout << "Seeded Event: Summer Music Festival (ID: " << events.back().eventId << ")\n";
        dataSeeded = true;
    }
    if (inventory.empty()) {
        std::cout << "Info: No inventory found. Seeding initial items.\n";
        inventory.emplace_back("Projector", 5, "HD Projector");
        std::cout << "Seeded Inventory: Projector (ID: " << inventory.back().itemId << ")\n";
        inventory.emplace_back("Chairs", 100, "Standard chairs");
        std::cout << "Seeded Inventory: Chairs (ID: " << inventory.back().itemId << ")\n";
        dataSeeded = true;
    }
    if (dataSeeded) {
        std::cout << "Initial data seeded. Saving to files...\n";
        saveData();
    }
}

void System::loadData() {
    loadUsers(); loadEvents(); loadInventory(); loadAttendees();
    int maxId = 0; for(const auto* u : users) if(u && u->getUserId() > maxId) maxId = u->getUserId(); User::initNextId(maxId +1);
    maxId = 0; for(const auto& e : events) if(e.eventId > maxId) maxId = e.eventId; Event::initNextId(maxId+1);
    maxId = 0; for(const auto& i : inventory) if(i.itemId > maxId) maxId = i.itemId; InventoryItem::initNextId(maxId+1);
    maxId = 0; for(const auto& a : allAttendees) if(a.attendeeId > maxId) maxId = a.attendeeId; Attendee::initNextId(maxId+1);
}
void System::saveData() { saveUsers(); saveEvents(); saveInventory(); saveAttendees(); }

void System::loadUsers() {
    std::ifstream inFile(USERS_FILE); if (!inFile) return;
    std::string line; while (std::getline(inFile, line)) if (!line.empty()) { User* u = User::fromString(line); if(u) users.push_back(u); }
    inFile.close();
}
void System::saveUsers() {
    std::ofstream outFile(USERS_FILE); if (!outFile) { std::cerr << "Err: USERS_FILE write.\n"; return; }
    for (const auto* user : users) if (user) outFile << user->toString() << std::endl;
    outFile.close();
}
void System::loadEvents() {
    std::ifstream inFile(EVENTS_FILE); if (!inFile) return;
    std::string line; while (std::getline(inFile, line)) if (!line.empty()) events.push_back(Event::fromString(line));
    inFile.close();
}
void System::saveEvents() {
    std::ofstream outFile(EVENTS_FILE); if (!outFile) { std::cerr << "Err: EVENTS_FILE write.\n"; return; }
    for (const auto& event : events) outFile << event.toString() << std::endl;
    outFile.close();
}
void System::loadInventory() {
    std::ifstream inFile(INVENTORY_FILE); if (!inFile) return;
    std::string line; while (std::getline(inFile, line)) if (!line.empty()) inventory.push_back(InventoryItem::fromString(line));
    inFile.close();
}
void System::saveInventory() {
    std::ofstream outFile(INVENTORY_FILE); if (!outFile) { std::cerr << "Err: INVENTORY_FILE write.\n"; return; }
    for (const auto& item : inventory) outFile << item.toString() << std::endl;
    outFile.close();
}
void System::loadAttendees() {
    std::ifstream inFile(ATTENDEES_FILE); if (!inFile) return;
    std::string line; while (std::getline(inFile, line)) if (!line.empty()) allAttendees.push_back(Attendee::fromString(line));
    inFile.close();
}
void System::saveAttendees() {
    std::ofstream outFile(ATTENDEES_FILE); if (!outFile) { std::cerr << "Err: ATTENDEES_FILE write.\n"; return; }
    for (const auto& attendee : allAttendees) outFile << attendee.toString() << std::endl;
    outFile.close();
}
bool System::usernameExists(const std::string& uname) const {
    for (const auto* user : users) if (user && user->getUsername() == uname) return true;
    return false;
}
void System::createUserAccount(const std::string& uname, const std::string& pwd, Role role) {
    if (usernameExists(uname)) { std::cout << "Username already exists.\n"; return;}
    if (pwd.length() < 6) { std::cout << "Password too short.\n"; return; }
    if (role == Role::ADMIN) users.push_back(new Admin(uname, pwd));
    else if (role == Role::REGULAR_USER) users.push_back(new RegularUser(uname, pwd));
    else { std::cout << "Invalid role.\n"; return; }
    std::cout << (role == Role::ADMIN ? "Admin" : "User") << " '" << uname << "' created (ID: " << users.back()->getUserId() << ").\n";
    saveUsers();
}
void System::publicRegisterNewUser() {
    std::cout << "\n--- Register New User ---\n";
    std::string uname = getStringInput("Username: ");
    std::string pwd = getStringInput("Password (min 6 chars): ");
    std::cout << "Account type: 1. Admin 2. Regular User\n";
    int rChoice = getIntInput("Choice (1-2): ");
    Role newRole = (rChoice == 1) ? Role::ADMIN : Role::REGULAR_USER;
    if (rChoice != 1 && rChoice != 2) newRole = Role::NONE; // Mark as invalid if choice is bad
    createUserAccount(uname, pwd, newRole);
}
void System::deleteUserAccount(const std::string& uname) {
    auto it = std::remove_if(users.begin(), users.end(), [&](User* u) {
        if (u && u->getUsername() == uname) {
            if (currentUser && currentUser->getUsername() == uname) { std::cout << "Cannot delete self.\n"; return false; }
            delete u; return true;
        }
        return false;
    });
    if (it != users.end()) { users.erase(it, users.end()); std::cout << "User '" << uname << "' deleted.\n"; saveUsers(); }
    else { std::cout << "User '" << uname << "' not found.\n"; }
}
User* System::findUserByUsername(const std::string& uname) {
    for (auto* user : users) if (user && user->getUsername() == uname) return user; return nullptr;
}
const User* System::findUserByUsername(const std::string& uname) const {
    for (const auto* user : users) if (user && user->getUsername() == uname) return user; return nullptr;
}
void System::listAllUsers() const {
    std::cout << "\n--- All Users ---\n"; if (users.empty()) { std::cout << "No users.\n"; return; }
    for (const auto* user : users) if (user) std::cout << "ID: " << user->getUserId() << ", User: " << user->getUsername() << ", Role: " << (user->getRole() == Role::ADMIN ? "Admin" : "User") << std::endl;
}
bool System::login() {
    std::cout << "\n--- Login ---\n";
    std::string uname = getStringInput("Username: "); std::string pwd = getStringInput("Password: ");
    for (User* u : users) if (u && u->getUsername() == uname && u->getPassword() == pwd) {
        currentUser = u; std::cout << "Login successful. Welcome, " << currentUser->getUsername() << "!\n"; return true;
    }
    std::cout << "Login failed.\n"; currentUser = nullptr; return false;
}
void System::logout() { if (currentUser) { std::cout << "Logging out " << currentUser->getUsername() << ".\n"; currentUser = nullptr; } }

Event* System::findEventById(int eventId) { for (auto& event : events) if (event.eventId == eventId) return &event; return nullptr; }
const Event* System::findEventById(int eventId) const { for (const auto& event : events) if (event.eventId == eventId) return &event; return nullptr; }

void System::createEvent() {
    std::cout << "\n--- Create Event ---\n";
    std::string name = getStringInput("Name: "); std::string date, time;
    while(true){ date = getStringInput("Date (YYYY-MM-DD): "); if(isValidDate(date)) break; std::cout << "Invalid date.\n"; }
    while(true){ time = getStringInput("Time (HH:MM): "); if(isValidTime(time)) break; std::cout << "Invalid time.\n"; }
    std::string loc = getStringInput("Location: "); std::string desc = getStringInput("Description: "); std::string cat = getStringInput("Category: ");
    events.emplace_back(name, date, time, loc, desc, cat);
    std::cout << "Event '" << name << "' created (ID: " << events.back().eventId << ").\n"; saveEvents();
}
void System::viewAllEvents(bool adminView) const {
    std::cout << "\n--- All Events ---\n"; if (events.empty()) { std::cout << "No events.\n"; return; }
    for (const auto& event : events) { event.displayDetails(*this); std::cout << "-------------------\n"; }
}
void System::searchEventsByNameOrDate() const { /* Simplified, implement as needed */ std::cout << "Search not fully implemented.\n"; }
void System::editEventDetails() { /* Simplified */ std::cout << "Edit Event not fully implemented.\n"; }
void System::deleteEvent() { /* Simplified */ std::cout << "Delete Event not fully implemented.\n"; }
void System::updateEventStatus() { /* Simplified */ std::cout << "Update Status not fully implemented.\n"; }
Attendee* System::findAttendeeInMasterList(int attendeeId) { for(auto& att : allAttendees) if(att.attendeeId == attendeeId) return &att; return nullptr; }
const Attendee* System::findAttendeeInMasterList(int attendeeId) const { for(const auto& att : allAttendees) if(att.attendeeId == attendeeId) return &att; return nullptr; }
void System::registerAttendeeForEvent() { /* Simplified */ std::cout << "Register Attendee not fully implemented.\n"; }
void System::cancelOwnRegistration() { /* Simplified */ std::cout << "Cancel Registration not fully implemented.\n"; }
void System::viewAttendeeListsPerEvent() const { /* Simplified */ std::cout << "View Attendee Lists not fully implemented.\n"; }
void System::checkInAttendeeForEvent() { /* Simplified */ std::cout << "Check-in not fully implemented.\n"; }
void System::generateAttendanceReportForEvent() const { /* Simplified */ std::cout << "Attendance Report not fully implemented.\n"; }
void System::exportAttendeeListForEventToFile() const { /* Simplified */ std::cout << "Export List not fully implemented.\n"; }
InventoryItem* System::findInventoryItemById(int itemId) { for(auto& item : inventory) if(item.itemId == itemId) return &item; return nullptr; }
const InventoryItem* System::findInventoryItemById(int itemId) const { for(const auto& item : inventory) if(item.itemId == itemId) return &item; return nullptr; }
InventoryItem* System::findInventoryItemByName(const std::string& name) { std::string ln = toLower(name); for(auto& item : inventory) if(toLower(item.name)==ln) return &item; return nullptr; }
const InventoryItem* System::findInventoryItemByName(const std::string& name) const { std::string ln = toLower(name); for(const auto& item : inventory) if(toLower(item.name)==ln) return &item; return nullptr; }
void System::addInventoryItem() { /* Simplified */ std::cout << "Add Inventory not fully implemented.\n"; }
void System::updateInventoryItemDetails() { /* Simplified */ std::cout << "Update Inventory not fully implemented.\n"; }
void System::viewAllInventoryItems() const { /* Simplified */ std::cout << "View All Inventory not fully implemented.\n"; }
void System::trackInventoryAllocationToEvent() { /* Simplified */ std::cout << "Track Allocation not fully implemented.\n"; }
void System::generateFullInventoryReport() const { /* Simplified */ std::cout << "Inventory Report not fully implemented.\n"; }
void System::exportAllEventsDataToFile() const { /* Simplified */ std::cout << "Export Events not fully implemented.\n"; }
void System::exportAllAttendeesDataToFile() const { /* Simplified */ std::cout << "Export Attendees not fully implemented.\n"; }
void System::exportAllInventoryDataToFile() const { /* Simplified */ std::cout << "Export Inventory not fully implemented.\n"; }
void System::updateCurrentLoggedInUserContactInfo() { /* Simplified */ std::cout << "Update Contact Info not fully implemented.\n"; }


// --- Admin::displayMenu Definition ---
void Admin::displayMenu(System& sys) {
    int choice;
    while (sys.currentUser == this) {
        std::cout << "\n--- Admin Menu (" << username << ") ---\n";
        std::cout << "1. User Accounts\n2. Events\n3. Attendees (Admin)\n4. Inventory\n5. Data Export\n6. Logout\n";
        choice = getIntInput("Choice (1-6): ");
        switch (choice) {
            case 1: adminUserManagementMenu(sys); break;
            case 2: adminEventManagementMenu(sys); break;
            case 3: adminAttendeeManagementMenu(sys); break;
            case 4: adminInventoryManagementMenu(sys); break;
            case 5: adminDataExportMenu(sys); break;
            case 6: sys.logout(); return;
            default: std::cout << "Invalid choice.\n";
        }
    }
}
void Admin::adminUserManagementMenu(System& sys) {
    int choice;
    std::cout << "\n  -- User Account Mgmt --\n  1. Create User\n  2. Delete User\n  3. List Users\n  4. Back\n";
    choice = getIntInput("  Choice (1-4): "); std::string uname, pwd; int rChoice;
    switch(choice) {
        case 1: uname = getStringInput("New Username: "); pwd = getStringInput("Password: ");
                std::cout << "Role: 1.Admin 2.Regular User\n"; rChoice = getIntInput("Role (1-2): ");
                sys.createUserAccount(uname, pwd, (rChoice==1 ? Role::ADMIN : Role::REGULAR_USER)); break;
        case 2: uname = getStringInput("Username to delete: "); sys.deleteUserAccount(uname); break;
        case 3: sys.listAllUsers(); break;
        case 4: return; default: std::cout << "Invalid.\n";
    }
}
void Admin::adminEventManagementMenu(System& sys) { std::cout << "Admin Event Menu TBD\n"; } // Simplified
void Admin::adminAttendeeManagementMenu(System& sys) { std::cout << "Admin Attendee Menu TBD\n"; } // Simplified
void Admin::adminInventoryManagementMenu(System& sys) { std::cout << "Admin Inventory Menu TBD\n"; } // Simplified
void Admin::adminDataExportMenu(System& sys) { std::cout << "Admin Data Export Menu TBD\n"; } // Simplified


// --- RegularUser::displayMenu Definition ---
void RegularUser::displayMenu(System& sys) {
    int choice;
    while (sys.currentUser == this) {
        std::cout << "\n--- User Menu (" << username << ") ---\n";
        std::cout << "1. Browse Events\n2. Search Events\n3. Register for Event\n4. Cancel Registration\n";
        std::cout << "5. View Attendee List\n6. Update Contact Info\n7. Change Password\n8. Logout\n";
        choice = getIntInput("Choice (1-8): ");
        switch (choice) {
            case 1: sys.viewAllEvents(); break;
            case 2: sys.searchEventsByNameOrDate(); break;
            case 3: sys.registerAttendeeForEvent(); break;
            case 4: sys.cancelOwnRegistration(); break;
            case 5: sys.viewAttendeeListsPerEvent(); break;
            case 6: sys.updateCurrentLoggedInUserContactInfo(); break;
            case 7: {
                std::cout << "--- Change Password ---\n";
                std::string currPass = getStringInput("Current Password: ");
                if (currPass != password) { std::cout << "Incorrect.\n"; break; }
                std::string newPass = getStringInput("New Password (min 6): ");
                std::string confPass = getStringInput("Confirm New Password: ");
                if (newPass != confPass) { std::cout << "Mismatch.\n"; break; }
                setPassword(newPass); // User base method
                if (sys.currentUser) sys.saveUsers(); else std::cout << "Session error.\n";
                break;
            }
            case 8: sys.logout(); return;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

// --- System::run Definition ---
void System::run() {
    loadData();
    seedInitialData();
    while (true) {
        if (!currentUser) {
            std::cout << "\n===== EMS Main Menu =====\n1. Login\n2. Register\n3. Exit\n";
            int choice = getIntInput("Choice (1-3): ");
            switch (choice) {
                case 1: login(); break;
                case 2: publicRegisterNewUser(); break;
                case 3: std::cout << "Exiting. Goodbye!\n"; return;
                default: std::cout << "Invalid choice.\n";
            }
        } else {
            currentUser->displayMenu(*this);
        }
    }
}

// --- Event::displayDetails Definition ---
void Event::displayDetails(const System& sys) const {
    std::cout << "Event ID: " << eventId << "\n  Name: " << name << "\n  Date: " << date << ", Time: " << time
              << "\n  Location: " << location << "\n  Category: " << category << "\n  Status: " << getStatusString()
              << "\n  Description: " << description << "\n  Attendees: " << attendeeIds.size() << "\n";
    if (!allocatedInventory.empty()) {
        std::cout << "  Inventory:\n";
        for (auto const& [invId, quantity] : allocatedInventory) {
            const InventoryItem* item = sys.findInventoryItemById(invId);
            std::cout << "    - " << (item ? item->name : "Unknown Item") << " (ID: " << invId << "), Qty: " << quantity << "\n";
        }
    } else {
        std::cout << "  No inventory allocated.\n";
    }
}

// --- Main Function ---
int main() {
    try {
        std::locale::global(std::locale(""));
        std::cout.imbue(std::locale());
        std::cin.imbue(std::locale());
    } catch (const std::runtime_error& e) {
        std::cerr << "Warn: Locale setup failed. " << e.what() << std::endl;
    }
    System eventManagementSystem;
    eventManagementSystem.run();
    return 0;
}