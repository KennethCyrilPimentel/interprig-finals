#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <limits>
#include <iomanip>
#include <sstream>
#include <map>

using namespace std;

// Utility functions
string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << 1900 + ltm->tm_year << "-" 
       << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
       << setw(2) << setfill('0') << ltm->tm_mday;
    return ss.str();
}

bool isValidDate(const string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    
    try {
        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));
        
        if (year < 2023 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        
        // Simple day validation (doesn't account for different month lengths)
        if (day < 1 || day > 31) return false;
        
        return true;
    } catch (...) {
        return false;
    }
}

bool isValidTime(const string& time) {
    if (time.length() != 5) return false;
    if (time[2] != ':') return false;
    
    try {
        int hour = stoi(time.substr(0, 2));
        int minute = stoi(time.substr(3, 2));
        
        if (hour < 0 || hour > 23) return false;
        if (minute < 0 || minute > 59) return false;
        
        return true;
    } catch (...) {
        return false;
    }
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Data structures
struct User {
    string username;
    string password;
    bool isAdmin;
};

struct Event {
    string name;
    string date;
    string time;
    string location;
    string description;
    string category;
    string status; // Upcoming, Ongoing, Completed, Canceled
};

struct Attendee {
    string name;
    string contactInfo;
    string eventName;
    bool checkedIn;
};

struct InventoryItem {
    string name;
    int quantity;
    string description;
    string allocatedEvent;
};

// System class
class EventManagementSystem {
private:
    vector<User> users;
    vector<Event> events;
    vector<Attendee> attendees;
    vector<InventoryItem> inventory;
    User currentUser;
    const string USER_FILE = "users.txt";
    const string EVENT_FILE = "events.txt";
    const string ATTENDEE_FILE = "attendees.txt";
    const string INVENTORY_FILE = "inventory.txt";

    void loadData() {
        // Load users
        ifstream userFile(USER_FILE);
        if (userFile.is_open()) {
            string line;
            while (getline(userFile, line)) {
                size_t pos1 = line.find(',');
                size_t pos2 = line.find(',', pos1 + 1);
                if (pos1 != string::npos && pos2 != string::npos) {
                    User user;
                    user.username = line.substr(0, pos1);
                    user.password = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    user.isAdmin = line.substr(pos2 + 1) == "1";
                    users.push_back(user);
                }
            }
            userFile.close();
        }

        // Load events
        ifstream eventFile(EVENT_FILE);
        if (eventFile.is_open()) {
            string line;
            while (getline(eventFile, line)) {
                size_t pos1 = line.find(',');
                size_t pos2 = line.find(',', pos1 + 1);
                size_t pos3 = line.find(',', pos2 + 1);
                size_t pos4 = line.find(',', pos3 + 1);
                size_t pos5 = line.find(',', pos4 + 1);
                size_t pos6 = line.find(',', pos5 + 1);
                if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos && 
                    pos4 != string::npos && pos5 != string::npos && pos6 != string::npos) {
                    Event event;
                    event.name = line.substr(0, pos1);
                    event.date = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    event.time = line.substr(pos2 + 1, pos3 - pos2 - 1);
                    event.location = line.substr(pos3 + 1, pos4 - pos3 - 1);
                    event.description = line.substr(pos4 + 1, pos5 - pos4 - 1);
                    event.category = line.substr(pos5 + 1, pos6 - pos5 - 1);
                    event.status = line.substr(pos6 + 1);
                    events.push_back(event);
                }
            }
            eventFile.close();
        }

        // Load attendees
        ifstream attendeeFile(ATTENDEE_FILE);
        if (attendeeFile.is_open()) {
            string line;
            while (getline(attendeeFile, line)) {
                size_t pos1 = line.find(',');
                size_t pos2 = line.find(',', pos1 + 1);
                size_t pos3 = line.find(',', pos2 + 1);
                if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos) {
                    Attendee attendee;
                    attendee.name = line.substr(0, pos1);
                    attendee.contactInfo = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    attendee.eventName = line.substr(pos2 + 1, pos3 - pos2 - 1);
                    attendee.checkedIn = line.substr(pos3 + 1) == "1";
                    attendees.push_back(attendee);
                }
            }
            attendeeFile.close();
        }

        // Load inventory
        ifstream inventoryFile(INVENTORY_FILE);
        if (inventoryFile.is_open()) {
            string line;
            while (getline(inventoryFile, line)) {
                size_t pos1 = line.find(',');
                size_t pos2 = line.find(',', pos1 + 1);
                size_t pos3 = line.find(',', pos2 + 1);
                if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos) {
                    InventoryItem item;
                    item.name = line.substr(0, pos1);
                    item.quantity = stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
                    item.description = line.substr(pos2 + 1, pos3 - pos2 - 1);
                    item.allocatedEvent = line.substr(pos3 + 1);
                    inventory.push_back(item);
                }
            }
            inventoryFile.close();
        }
    }

    void saveData() {
        // Save users
        ofstream userFile(USER_FILE);
        if (userFile.is_open()) {
            for (const User& user : users) {
                userFile << user.username << "," << user.password << "," << (user.isAdmin ? "1" : "0") << "\n";
            }
            userFile.close();
        }

        // Save events
        ofstream eventFile(EVENT_FILE);
        if (eventFile.is_open()) {
            for (const Event& event : events) {
                eventFile << event.name << "," << event.date << "," << event.time << ","
                           << event.location << "," << event.description << ","
                           << event.category << "," << event.status << "\n";
            }
            eventFile.close();
        }

        // Save attendees
        ofstream attendeeFile(ATTENDEE_FILE);
        if (attendeeFile.is_open()) {
            for (const Attendee& attendee : attendees) {
                attendeeFile << attendee.name << "," << attendee.contactInfo << ","
                            << attendee.eventName << "," << (attendee.checkedIn ? "1" : "0") << "\n";
            }
            attendeeFile.close();
        }

        // Save inventory
        ofstream inventoryFile(INVENTORY_FILE);
        if (inventoryFile.is_open()) {
            for (const InventoryItem& item : inventory) {
                inventoryFile << item.name << "," << item.quantity << ","
                             << item.description << "," << item.allocatedEvent << "\n";
            }
            inventoryFile.close();
        }
    }

    bool usernameExists(const string& username) {
        for (const User& user : users) {
            if (user.username == username) {
                return true;
            }
        }
        return false;
    }

    bool eventExists(const string& eventName) {
        for (const Event& event : events) {
            if (event.name == eventName) {
                return true;
            }
        }
        return false;
    }

    bool attendeeExists(const string& attendeeName, const string& eventName) {
        for (const Attendee& attendee : attendees) {
            if (attendee.name == attendeeName && attendee.eventName == eventName) {
                return true;
            }
        }
        return false;
    }

    bool inventoryItemExists(const string& itemName) {
        for (const InventoryItem& item : inventory) {
            if (item.name == itemName) {
                return true;
            }
        }
        return false;
    }

    void displayEvent(const Event& event) {
        cout << "\nEvent Name: " << event.name << "\n";
        cout << "Date: " << event.date << "\n";
        cout << "Time: " << event.time << "\n";
        cout << "Location: " << event.location << "\n";
        cout << "Description: " << event.description << "\n";
        cout << "Category: " << event.category << "\n";
        cout << "Status: " << event.status << "\n";
    }

    void displayAttendee(const Attendee& attendee) {
        cout << "\nAttendee Name: " << attendee.name << "\n";
        cout << "Contact Info: " << attendee.contactInfo << "\n";
        cout << "Event: " << attendee.eventName << "\n";
        cout << "Checked In: " << (attendee.checkedIn ? "Yes" : "No") << "\n";
    }

    void displayInventoryItem(const InventoryItem& item) {
        cout << "\nItem Name: " << item.name << "\n";
        cout << "Quantity: " << item.quantity << "\n";
        cout << "Description: " << item.description << "\n";
        cout << "Allocated Event: " << (item.allocatedEvent.empty() ? "None" : item.allocatedEvent) << "\n";
    }

public:
    EventManagementSystem() {
        loadData();
    }

    ~EventManagementSystem() {
        saveData();
    }

    void login() {
        string username, password;
        cout << "\n=== Login ===\n";
        cout << "Username: ";
        getline(cin, username);
        cout << "Password: ";
        getline(cin, password);

        for (const User& user : users) {
            if (user.username == username && user.password == password) {
                currentUser = user;
                cout << "\nLogin successful! Welcome, " << username << ".\n";
                return;
            }
        }

        cout << "\nInvalid username or password. Please try again.\n";
        login();
    }

    void registerUser() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can register new users.\n";
            return;
        }

        User newUser;
        cout << "\n=== Register New User ===\n";
        cout << "Username: ";
        getline(cin, newUser.username);

        if (usernameExists(newUser.username)) {
            cout << "\nUsername already exists. Please choose a different username.\n";
            return;
        }

        cout << "Password: ";
        getline(cin, newUser.password);

        string isAdminStr;
        cout << "Is this user an admin? (y/n): ";
        getline(cin, isAdminStr);
        newUser.isAdmin = (isAdminStr == "y" || isAdminStr == "Y");

        users.push_back(newUser);
        saveData();
        cout << "\nUser registered successfully!\n";
    }

    void createEvent() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can create events.\n";
            return;
        }

        Event newEvent;
        cout << "\n=== Create New Event ===\n";
        
        cout << "Event Name: ";
        getline(cin, newEvent.name);
        if (eventExists(newEvent.name)) {
            cout << "\nAn event with this name already exists.\n";
            return;
        }

        cout << "Date (YYYY-MM-DD): ";
        getline(cin, newEvent.date);
        if (!isValidDate(newEvent.date)) {
            cout << "\nInvalid date format. Please use YYYY-MM-DD.\n";
            return;
        }

        cout << "Time (HH:MM): ";
        getline(cin, newEvent.time);
        if (!isValidTime(newEvent.time)) {
            cout << "\nInvalid time format. Please use HH:MM.\n";
            return;
        }

        cout << "Location: ";
        getline(cin, newEvent.location);

        cout << "Description: ";
        getline(cin, newEvent.description);

        cout << "Category (Conference, Social, etc.): ";
        getline(cin, newEvent.category);

        newEvent.status = "Upcoming";
        events.push_back(newEvent);
        saveData();
        cout << "\nEvent created successfully!\n";
    }

    void viewEvents() {
        cout << "\n=== Events ===\n";
        if (events.empty()) {
            cout << "No events found.\n";
            return;
        }

        for (const Event& event : events) {
            displayEvent(event);
        }
    }

    void searchEvents() {
        string keyword;
        cout << "\n=== Search Events ===\n";
        cout << "Enter search keyword (name or date): ";
        getline(cin, keyword);

        bool found = false;
        for (const Event& event : events) {
            if (event.name.find(keyword) != string::npos || event.date.find(keyword) != string::npos) {
                displayEvent(event);
                found = true;
            }
        }

        if (!found) {
            cout << "No events matching the search criteria.\n";
        }
    }

    void editEvent() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can edit events.\n";
            return;
        }

        string eventName;
        cout << "\n=== Edit Event ===\n";
        cout << "Enter event name to edit: ";
        getline(cin, eventName);

        for (Event& event : events) {
            if (event.name == eventName) {
                displayEvent(event);
                
                cout << "\nEnter new details (leave blank to keep current value):\n";
                
                string input;
                cout << "Date (YYYY-MM-DD) [" << event.date << "]: ";
                getline(cin, input);
                if (!input.empty()) {
                    if (isValidDate(input)) {
                        event.date = input;
                    } else {
                        cout << "Invalid date format. Date not updated.\n";
                    }
                }

                cout << "Time (HH:MM) [" << event.time << "]: ";
                getline(cin, input);
                if (!input.empty()) {
                    if (isValidTime(input)) {
                        event.time = input;
                    } else {
                        cout << "Invalid time format. Time not updated.\n";
                    }
                }

                cout << "Location [" << event.location << "]: ";
                getline(cin, input);
                if (!input.empty()) event.location = input;

                cout << "Description [" << event.description << "]: ";
                getline(cin, input);
                if (!input.empty()) event.description = input;

                cout << "Category [" << event.category << "]: ";
                getline(cin, input);
                if (!input.empty()) event.category = input;

                cout << "Status (Upcoming/Ongoing/Completed/Canceled) [" << event.status << "]: ";
                getline(cin, input);
                if (!input.empty()) {
                    if (input == "Upcoming" || input == "Ongoing" || input == "Completed" || input == "Canceled") {
                        event.status = input;
                    } else {
                        cout << "Invalid status. Status must be one of: Upcoming, Ongoing, Completed, Canceled\n";
                    }
                }

                saveData();
                cout << "\nEvent updated successfully!\n";
                return;
            }
        }

        cout << "\nEvent not found.\n";
    }

    void deleteEvent() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can delete events.\n";
            return;
        }

        string eventName;
        cout << "\n=== Delete Event ===\n";
        cout << "Enter event name to delete: ";
        getline(cin, eventName);

        for (auto it = events.begin(); it != events.end(); ++it) {
            if (it->name == eventName) {
                // Remove attendees for this event
                attendees.erase(
                    remove_if(attendees.begin(), attendees.end(),
                        [&eventName](const Attendee& a) { return a.eventName == eventName; }),
                    attendees.end()
                );

                // Unallocate inventory for this event
                for (InventoryItem& item : inventory) {
                    if (item.allocatedEvent == eventName) {
                        item.allocatedEvent = "";
                    }
                }

                events.erase(it);
                saveData();
                cout << "\nEvent deleted successfully!\n";
                return;
            }
        }

        cout << "\nEvent not found.\n";
    }

    void registerAttendee() {
        string eventName;
        cout << "\n=== Register Attendee ===\n";
        cout << "Enter event name: ";
        getline(cin, eventName);

        if (!eventExists(eventName)) {
            cout << "\nEvent not found.\n";
            return;
        }

        Attendee newAttendee;
        newAttendee.eventName = eventName;

        cout << "Attendee Name: ";
        getline(cin, newAttendee.name);

        if (attendeeExists(newAttendee.name, eventName)) {
            cout << "\nThis attendee is already registered for this event.\n";
            return;
        }

        cout << "Contact Info: ";
        getline(cin, newAttendee.contactInfo);

        newAttendee.checkedIn = false;
        attendees.push_back(newAttendee);
        saveData();
        cout << "\nAttendee registered successfully!\n";
    }

    void viewAttendees() {
        string eventName;
        cout << "\n=== View Attendees ===\n";
        cout << "Enter event name: ";
        getline(cin, eventName);

        if (!eventExists(eventName)) {
            cout << "\nEvent not found.\n";
            return;
        }

        bool found = false;
        for (const Attendee& attendee : attendees) {
            if (attendee.eventName == eventName) {
                displayAttendee(attendee);
                found = true;
            }
        }

        if (!found) {
            cout << "No attendees found for this event.\n";
        }
    }

    void checkInAttendee() {
        string eventName, attendeeName;
        cout << "\n=== Check In Attendee ===\n";
        cout << "Enter event name: ";
        getline(cin, eventName);

        if (!eventExists(eventName)) {
            cout << "\nEvent not found.\n";
            return;
        }

        cout << "Enter attendee name: ";
        getline(cin, attendeeName);

        for (Attendee& attendee : attendees) {
            if (attendee.name == attendeeName && attendee.eventName == eventName) {
                if (attendee.checkedIn) {
                    cout << "\nAttendee is already checked in.\n";
                } else {
                    attendee.checkedIn = true;
                    saveData();
                    cout << "\nAttendee checked in successfully!\n";
                }
                return;
            }
        }

        cout << "\nAttendee not found for this event.\n";
    }

    void exportAttendees() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can export attendee lists.\n";
            return;
        }

        string eventName;
        cout << "\n=== Export Attendee List ===\n";
        cout << "Enter event name: ";
        getline(cin, eventName);

        if (!eventExists(eventName)) {
            cout << "\nEvent not found.\n";
            return;
        }

        string filename = eventName + "_attendees.txt";
        ofstream outFile(filename);
        if (!outFile.is_open()) {
            cout << "\nError creating export file.\n";
            return;
        }

        outFile << "Attendees for event: " << eventName << "\n";
        outFile << "----------------------------------------\n";
        outFile << "Name\t\tContact Info\tChecked In\n";
        outFile << "----------------------------------------\n";

        int count = 0;
        for (const Attendee& attendee : attendees) {
            if (attendee.eventName == eventName) {
                outFile << attendee.name << "\t" << attendee.contactInfo << "\t"
                         << (attendee.checkedIn ? "Yes" : "No") << "\n";
                count++;
            }
        }

        outFile << "----------------------------------------\n";
        outFile << "Total attendees: " << count << "\n";
        outFile.close();

        cout << "\nAttendee list exported to " << filename << " successfully!\n";
    }

    void generateAttendanceReport() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can generate attendance reports.\n";
            return;
        }

        cout << "\n=== Attendance Report ===\n";
        cout << "Event Name\t\tTotal Attendees\tChecked In\n";
        cout << "------------------------------------------------\n";

        map<string, pair<int, int>> eventStats; // eventName -> <total, checkedIn>

        for (const Attendee& attendee : attendees) {
            eventStats[attendee.eventName].first++;
            if (attendee.checkedIn) {
                eventStats[attendee.eventName].second++;
            }
        }

        for (const auto& [eventName, stats] : eventStats) {
            cout << eventName << "\t\t" << stats.first << "\t\t" << stats.second << "\n";
        }

        cout << "------------------------------------------------\n";
    }

    void addInventoryItem() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can add inventory items.\n";
            return;
        }

        InventoryItem newItem;
        cout << "\n=== Add Inventory Item ===\n";
        cout << "Item Name: ";
        getline(cin, newItem.name);

        if (inventoryItemExists(newItem.name)) {
            cout << "\nAn item with this name already exists.\n";
            return;
        }

        cout << "Quantity: ";
        cin >> newItem.quantity;
        clearInputBuffer();

        if (newItem.quantity < 0) {
            cout << "\nQuantity cannot be negative.\n";
            return;
        }

        cout << "Description: ";
        getline(cin, newItem.description);

        newItem.allocatedEvent = "";
        inventory.push_back(newItem);
        saveData();
        cout << "\nInventory item added successfully!\n";
    }

    void viewInventory() {
        cout << "\n=== Inventory ===\n";
        if (inventory.empty()) {
            cout << "No inventory items found.\n";
            return;
        }

        for (const InventoryItem& item : inventory) {
            displayInventoryItem(item);
        }
    }

    void allocateInventory() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can allocate inventory.\n";
            return;
        }

        string itemName, eventName;
        cout << "\n=== Allocate Inventory ===\n";
        cout << "Enter item name: ";
        getline(cin, itemName);

        cout << "Enter event name: ";
        getline(cin, eventName);

        if (!eventExists(eventName)) {
            cout << "\nEvent not found.\n";
            return;
        }

        for (InventoryItem& item : inventory) {
            if (item.name == itemName) {
                if (!item.allocatedEvent.empty()) {
                    cout << "\nThis item is already allocated to " << item.allocatedEvent << ".\n";
                    cout << "Do you want to reallocate it? (y/n): ";
                    string choice;
                    getline(cin, choice);
                    if (choice != "y" && choice != "Y") {
                        return;
                    }
                }
                item.allocatedEvent = eventName;
                saveData();
                cout << "\nInventory allocated successfully!\n";
                return;
            }
        }

        cout << "\nInventory item not found.\n";
    }

    void generateInventoryReport() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can generate inventory reports.\n";
            return;
        }

        cout << "\n=== Inventory Report ===\n";
        cout << "Item Name\tQuantity\tAllocated To\n";
        cout << "------------------------------------------------\n";

        for (const InventoryItem& item : inventory) {
            cout << item.name << "\t\t" << item.quantity << "\t\t"
                 << (item.allocatedEvent.empty() ? "None" : item.allocatedEvent) << "\n";
        }

        cout << "------------------------------------------------\n";
        cout << "Total items: " << inventory.size() << "\n";
    }

    void exportData() {
        if (!currentUser.isAdmin) {
            cout << "\nOnly admins can export data.\n";
            return;
        }

        cout << "\n=== Export Data ===\n";
        cout << "1. Export Events\n";
        cout << "2. Export Attendees\n";
        cout << "3. Export Inventory\n";
        cout << "4. Cancel\n";
        cout << "Enter choice: ";

        int choice;
        cin >> choice;
        clearInputBuffer();

        string filename;
        switch (choice) {
            case 1: {
                filename = "events_export_" + getCurrentDate() + ".txt";
                ofstream outFile(filename);
                if (!outFile.is_open()) {
                    cout << "\nError creating export file.\n";
                    return;
                }

                outFile << "Events Export - " << getCurrentDate() << "\n";
                outFile << "------------------------------------------------\n";
                for (const Event& event : events) {
                    outFile << "Name: " << event.name << "\n";
                    outFile << "Date: " << event.date << "\n";
                    outFile << "Time: " << event.time << "\n";
                    outFile << "Location: " << event.location << "\n";
                    outFile << "Description: " << event.description << "\n";
                    outFile << "Category: " << event.category << "\n";
                    outFile << "Status: " << event.status << "\n";
                    outFile << "------------------------------------------------\n";
                }
                outFile.close();
                cout << "\nEvents exported to " << filename << " successfully!\n";
                break;
            }
            case 2: {
                filename = "attendees_export_" + getCurrentDate() + ".txt";
                ofstream outFile(filename);
                if (!outFile.is_open()) {
                    cout << "\nError creating export file.\n";
                    return;
                }

                outFile << "Attendees Export - " << getCurrentDate() << "\n";
                outFile << "------------------------------------------------\n";
                for (const Attendee& attendee : attendees) {
                    outFile << "Name: " << attendee.name << "\n";
                    outFile << "Contact Info: " << attendee.contactInfo << "\n";
                    outFile << "Event: " << attendee.eventName << "\n";
                    outFile << "Checked In: " << (attendee.checkedIn ? "Yes" : "No") << "\n";
                    outFile << "------------------------------------------------\n";
                }
                outFile.close();
                cout << "\nAttendees exported to " << filename << " successfully!\n";
                break;
            }
            case 3: {
                filename = "inventory_export_" + getCurrentDate() + ".txt";
                ofstream outFile(filename);
                if (!outFile.is_open()) {
                    cout << "\nError creating export file.\n";
                    return;
                }

                outFile << "Inventory Export - " << getCurrentDate() << "\n";
                outFile << "------------------------------------------------\n";
                for (const InventoryItem& item : inventory) {
                    outFile << "Name: " << item.name << "\n";
                    outFile << "Quantity: " << item.quantity << "\n";
                    outFile << "Description: " << item.description << "\n";
                    outFile << "Allocated Event: " << (item.allocatedEvent.empty() ? "None" : item.allocatedEvent) << "\n";
                    outFile << "------------------------------------------------\n";
                }
                outFile.close();
                cout << "\nInventory exported to " << filename << " successfully!\n";
                break;
            }
            case 4:
                cout << "\nExport canceled.\n";
                break;
            default:
                cout << "\nInvalid choice.\n";
        }
    }

    void showAdminMenu() {
        while (true) {
            cout << "\n=== Admin Menu ===\n";
            cout << "1. Create Event\n";
            cout << "2. View Events\n";
            cout << "3. Search Events\n";
            cout << "4. Edit Event\n";
            cout << "5. Delete Event\n";
            cout << "6. Register Attendee\n";
            cout << "7. View Attendees\n";
            cout << "8. Check In Attendee\n";
            cout << "9. Export Attendee List\n";
            cout << "10. Generate Attendance Report\n";
            cout << "11. Add Inventory Item\n";
            cout << "12. View Inventory\n";
            cout << "13. Allocate Inventory\n";
            cout << "14. Generate Inventory Report\n";
            cout << "15. Export Data\n";
            cout << "16. Register New User\n";
            cout << "17. Logout\n";
            cout << "Enter choice: ";

            int choice;
            cin >> choice;
            clearInputBuffer();

            switch (choice) {
                case 1: createEvent(); break;
                case 2: viewEvents(); break;
                case 3: searchEvents(); break;
                case 4: editEvent(); break;
                case 5: deleteEvent(); break;
                case 6: registerAttendee(); break;
                case 7: viewAttendees(); break;
                case 8: checkInAttendee(); break;
                case 9: exportAttendees(); break;
                case 10: generateAttendanceReport(); break;
                case 11: addInventoryItem(); break;
                case 12: viewInventory(); break;
                case 13: allocateInventory(); break;
                case 14: generateInventoryReport(); break;
                case 15: exportData(); break;
                case 16: registerUser(); break;
                case 17: return;
                default: cout << "\nInvalid choice. Please try again.\n";
            }
        }
    }

    void showUserMenu() {
        while (true) {
            cout << "\n=== User Menu ===\n";
            cout << "1. View Events\n";
            cout << "2. Search Events\n";
            cout << "3. Register for Event\n";
            cout << "4. View Your Registrations\n";
            cout << "5. Update Contact Info\n";
            cout << "6. Logout\n";
            cout << "Enter choice: ";

            int choice;
            cin >> choice;
            clearInputBuffer();

            switch (choice) {
                case 1: viewEvents(); break;
                case 2: searchEvents(); break;
                case 3: registerAttendee(); break;
                case 4: {
                    cout << "\n=== Your Registrations ===\n";
                    bool found = false;
                    for (const Attendee& attendee : attendees) {
                        if (attendee.name == currentUser.username) {
                            displayAttendee(attendee);
                            found = true;
                        }
                    }
                    if (!found) {
                        cout << "You are not registered for any events.\n";
                    }
                    break;
                }
                case 5: {
                    cout << "\n=== Update Contact Info ===\n";
                    bool found = false;
                    for (Attendee& attendee : attendees) {
                        if (attendee.name == currentUser.username) {
                            cout << "Current contact info: " << attendee.contactInfo << "\n";
                            cout << "Enter new contact info: ";
                            getline(cin, attendee.contactInfo);
                            saveData();
                            cout << "Contact info updated successfully!\n";
                            found = true;
                        }
                    }
                    if (!found) {
                        cout << "You are not registered for any events yet.\n";
                    }
                    break;
                }
                case 6: return;
                default: cout << "\nInvalid choice. Please try again.\n";
            }
        }
    }

    void run() {
        // Add a default admin if no users exist
        if (users.empty()) {
            User admin;
            admin.username = "admin";
            admin.password = "admin123";
            admin.isAdmin = true;
            users.push_back(admin);
            saveData();
            cout << "Default admin account created (username: admin, password: admin123)\n";
        }

        while (true) {
            cout << "\n=== Event Management System ===\n";
            cout << "1. Login\n";
            cout << "2. Exit\n";
            cout << "Enter choice: ";

            int choice;
            cin >> choice;
            clearInputBuffer();

            switch (choice) {
                case 1:
                    login();
                    if (currentUser.isAdmin) {
                        showAdminMenu();
                    } else {
                        showUserMenu();
                    }
                    currentUser = User(); // Clear current user on logout
                    break;
                case 2:
                    cout << "\nExiting system. Goodbye!\n";
                    return;
                default:
                    cout << "\nInvalid choice. Please try again.\n";
            }
        }
    }
};

int main() {
    EventManagementSystem system;
    system.run();
    return 0;
}