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
        // Trim leading/trailing whitespace (optional but good practice)
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
        std::cin.clear(); // Clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
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

// Basic date validation (format YYYY-MM-DD)
bool isValidDate(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100) return false; // Basic year range
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false; // Simplistic day check, not month-specific
        // More complex validation (e.g., days in month, leap year) can be added
    } catch (const std::exception&) {
        return false; // Conversion to int failed
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
        return false; // Conversion to int failed
    }
    return true;
}


// --- Class Definitions ---

// ** User Class (Abstract Base Class) **
class User {
protected:
    std::string username;
    std::string password; // In a real system, HASH THIS!
    Role role;
    int userId; 

    static int nextUserId; // Static member to generate unique IDs

public:
    // Constructor for new users
    User(std::string uname, std::string pwd, Role r)
        : username(std::move(uname)), password(std::move(pwd)), role(r) {
        userId = nextUserId++;
    }
    // Constructor for loading users (ID is provided)
    User(int id, std::string uname, std::string pwd, Role r)
        : userId(id), username(std::move(uname)), password(std::move(pwd)), role(r) {
        if (id >= nextUserId) { // Ensure nextUserId is always greater than any loaded ID
            nextUserId = id + 1;
        }
    }
    virtual ~User() {} // Virtual destructor for proper cleanup of derived classes

    // Getters
    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; } // Used for authentication logic
    Role getRole() const { return role; }
    int getUserId() const { return userId; }

    // Setters (primarily for profile updates by the user or admin)
    void setPassword(const std::string& newPassword) {
        if (newPassword.length() < 6) { // Basic password strength validation
            std::cout << "Password must be at least 6 characters long.\n";
            return;
        }
        password = newPassword;
        std::cout << "Password updated successfully.\n";
    }
    // Username change is typically more complex (uniqueness checks, etc.) and often disallowed or restricted.
    // void setUsername(const std::string& newUsername) { username = newUsername; }


    // Pure virtual function for displaying role-specific menus
    virtual void displayMenu(System& sys) = 0;

    // Method to convert user data to a string for file storage
    virtual std::string toString() const {
        std::stringstream ss;
        ss << userId << "," << username << "," << password << "," << static_cast<int>(role);
        return ss.str();
    }

    // Static factory method to create User objects from a string (e.g., when loading from file)
    // Implementation requires Admin and RegularUser to be defined, so it's defined later.
    static User* fromString(const std::string& str); 

    static void initNextId(int id) { if (id >= nextUserId) nextUserId = id + 1;}
};
int User::nextUserId = 1; // Initialize static member


// ** Attendee Class **
// Represents an individual attending an event. Can be a registered system user or an external guest.
class Attendee {
public:
    int attendeeId;           // Unique ID for this attendee entry. If a system user, could be their userId.
    std::string name;
    std::string contactInfo;  // e.g., email or phone number
    int eventIdRegisteredFor; // ID of the event this specific attendee record is for (can be 0 if generic profile)
    bool isCheckedIn;

    static int nextAttendeeId; // Static member for generating unique IDs

    // Constructor for new attendees (e.g., guests registered by admin, or when a user first provides contact info)
    Attendee(std::string n, std::string contact, int eventId)
        : name(std::move(n)), contactInfo(std::move(contact)), eventIdRegisteredFor(eventId), isCheckedIn(false) {
        attendeeId = nextAttendeeId++;
    }

    // Constructor for loading attendees from file (ID is provided)
    Attendee(int id, std::string n, std::string contact, int eventId, bool checkedInStatus)
        : attendeeId(id), name(std::move(n)), contactInfo(std::move(contact)), 
          eventIdRegisteredFor(eventId), isCheckedIn(checkedInStatus) {
        if (id >= nextAttendeeId) {
            nextAttendeeId = id + 1;
        }
    }
    
    // Method to mark an attendee as checked-in for their registered event
    void checkIn() {
        if (!isCheckedIn) {
            isCheckedIn = true;
            std::cout << name << " checked in successfully for event ID " << eventIdRegisteredFor << ".\n";
        } else {
            std::cout << name << " is already checked in for event ID " << eventIdRegisteredFor << ".\n";
        }
    }

    // Displays details of the attendee
    void displayDetails() const {
        std::cout << "Attendee ID: " << attendeeId
                  << ", Name: " << name
                  << ", Contact: " << contactInfo
                  << ", Registered for Event ID: " << (eventIdRegisteredFor == 0 ? "N/A (Profile)" : std::to_string(eventIdRegisteredFor))
                  << ", Checked-in: " << (isCheckedIn ? "Yes" : "No") << std::endl;
    }

    // Converts attendee data to a string for file storage
    std::string toString() const {
        std::stringstream ss;
        ss << attendeeId << "," << name << "," << contactInfo << "," << eventIdRegisteredFor << "," << (isCheckedIn ? "1" : "0");
        return ss.str();
    }

    // Static factory method to create Attendee objects from a string
    static Attendee fromString(const std::string& str) {
        std::stringstream ss(str);
        std::string segment;
        int id, eventId;
        std::string name, contact;
        bool checkedIn;

        std::getline(ss, segment, ','); id = std::stoi(segment);
        std::getline(ss, name, ','); // No std::move needed here
        std::getline(ss, contact, ',');
        std::getline(ss, segment, ','); eventId = std::stoi(segment);
        std::getline(ss, segment, ','); checkedIn = (segment == "1");
        
        return Attendee(id, name, contact, eventId, checkedIn);
    }
    static void initNextId(int id) { if (id >= nextAttendeeId) nextAttendeeId = id + 1;}
};
int Attendee::nextAttendeeId = 1;


// ** InventoryItem Class **
// Represents an item in the event inventory (e.g., chairs, projectors).
class InventoryItem {
public:
    int itemId;
    std::string name;
    int totalQuantity;        // Total number of this item owned
    int allocatedQuantity;    // Number of this item currently allocated to various events
    std::string description;

    static int nextItemId; // Static member for unique IDs

    // Constructor for new inventory items
    InventoryItem(std::string n, int qty, std::string desc)
        : name(std::move(n)), totalQuantity(qty), allocatedQuantity(0), description(std::move(desc)) {
        itemId = nextItemId++;
    }

    // Constructor for loading items from file
    InventoryItem(int id, std::string n, int totalQty, int allocQty, std::string desc)
        : itemId(id), name(std::move(n)), totalQuantity(totalQty), allocatedQuantity(allocQty), description(std::move(desc)) {
        if (id >= nextItemId) {
            nextItemId = id + 1;
        }
    }

    // Calculates the currently available quantity of the item
    int getAvailableQuantity() const {
        return totalQuantity - allocatedQuantity;
    }

    // Attempts to allocate a certain quantity of this item
    bool allocate(int quantityToAllocate) {
        if (quantityToAllocate <= 0) {
            std::cout << "Error: Allocation quantity must be positive.\n";
            return false;
        }
        if (quantityToAllocate <= getAvailableQuantity()) {
            allocatedQuantity += quantityToAllocate;
            // std::cout << quantityToAllocate << " of " << name << " allocated. (Now allocated: " << allocatedQuantity << ")\n";
            return true;
        }
        std::cout << "Error: Not enough '" << name << "' available to allocate " << quantityToAllocate 
                  << ". Available: " << getAvailableQuantity() << std::endl;
        return false;
    }

    // Attempts to deallocate a certain quantity of this item
    bool deallocate(int quantityToDeallocate) {
        if (quantityToDeallocate <= 0) {
            std::cout << "Error: Deallocation quantity must be positive.\n";
            return false;
        }
        if (quantityToDeallocate <= allocatedQuantity) {
            allocatedQuantity -= quantityToDeallocate;
            // std::cout << quantityToDeallocate << " of " << name << " deallocated. (Now allocated: " << allocatedQuantity << ")\n";
            return true;
        }
        std::cout << "Error: Cannot deallocate " << quantityToDeallocate << " of '" << name 
                  << "'. Only " << allocatedQuantity << " are currently allocated.\n";
        return false;
    }
    
    // Updates the total quantity of the item.
    void setTotalQuantity(int newTotalQuantity) {
        if (newTotalQuantity < 0) {
             std::cout << "Error: Total quantity cannot be negative.\n"; return;
        }
        if (newTotalQuantity < allocatedQuantity) {
            std::cout << "Error: New total quantity (" << newTotalQuantity 
                      << ") cannot be less than currently allocated quantity (" << allocatedQuantity << ").\n"
                      << "Deallocate from events first or increase total quantity.\n";
            return;
        }
        totalQuantity = newTotalQuantity;
        std::cout << "Total quantity for '" << name << "' updated to " << totalQuantity << ".\n";
    }

    // Displays details of the inventory item
    void displayDetails() const {
        std::cout << "Item ID: " << itemId << ", Name: " << name
                  << ", Total: " << totalQuantity
                  << ", Allocated: " << allocatedQuantity
                  << ", Available: " << getAvailableQuantity()
                  << ", Desc: " << description << std::endl;
    }

    // Converts item data to a string for file storage
    std::string toString() const {
        std::stringstream ss;
        ss << itemId << "," << name << "," << totalQuantity << "," << allocatedQuantity << "," << description;
        return ss.str();
    }

    // Static factory method to create InventoryItem objects from a string
    static InventoryItem fromString(const std::string& str) {
        std::stringstream ss(str);
        std::string segment;
        int id, totalQty, allocQty;
        std::string name, desc;

        std::getline(ss, segment, ','); id = std::stoi(segment);
        std::getline(ss, name, ',');
        std::getline(ss, segment, ','); totalQty = std::stoi(segment);
        std::getline(ss, segment, ','); allocQty = std::stoi(segment);
        std::getline(ss, desc); // Read the rest of the line for description

        return InventoryItem(id, name, totalQty, allocQty, desc);
    }
    static void initNextId(int id) { if (id >= nextItemId) nextItemId = id + 1;}
};
int InventoryItem::nextItemId = 1;


// ** Event Class **
// Represents an event being managed by the system.
class Event {
public:
    int eventId;
    std::string name;
    std::string date;         // Format: YYYY-MM-DD
    std::string time;         // Format: HH:MM (24-hour)
    std::string location;
    std::string description;
    std::string category;     // e.g., "Conference", "Social", "Workshop"
    EventStatus status;
    std::vector<int> attendeeIds; // Stores IDs of attendees (from System's allAttendees list) registered for this event
    std::map<int, int> allocatedInventory; // Maps itemId to quantity allocated for this event

    static int nextEventId; // Static member for unique IDs

    // Constructor for new events
    Event(std::string n, std::string d, std::string t, std::string loc, std::string desc, std::string cat)
        : name(std::move(n)), date(std::move(d)), time(std::move(t)), location(std::move(loc)), 
          description(std::move(desc)), category(std::move(cat)), status(EventStatus::UPCOMING) {
        eventId = nextEventId++;
    }
    
    // Constructor for loading events from file
    Event(int id, std::string n, std::string d, std::string t, std::string loc, 
          std::string desc, std::string cat, EventStatus stat)
        : eventId(id), name(std::move(n)), date(std::move(d)), time(std::move(t)), location(std::move(loc)),
          description(std::move(desc)), category(std::move(cat)), status(stat) {
        if (id >= nextEventId) {
            nextEventId = id + 1;
        }
    }

    // Adds an attendee's ID to this event's registration list
    void addAttendee(int attendeeId) {
        // Prevent duplicate registrations for the same event
        if (std::find(attendeeIds.begin(), attendeeIds.end(), attendeeId) == attendeeIds.end()) {
            attendeeIds.push_back(attendeeId);
        } else {
            std::cout << "Info: Attendee with ID " << attendeeId << " is already registered for event '" << name << "'.\n";
        }
    }

    // Removes an attendee's ID from this event's registration list
    void removeAttendee(int attendeeId) {
        auto it = std::find(attendeeIds.begin(), attendeeIds.end(), attendeeId);
        if (it != attendeeIds.end()) {
            attendeeIds.erase(it);
            // std::cout << "Attendee with ID " << attendeeId << " removed from event '" << name << "'.\n";
        } else {
            // std::cout << "Info: Attendee with ID " << attendeeId << " not found in event '" << name << "'.\n";
        }
    }

    // Records allocation of an inventory item to this event
    void allocateInventoryItem(int itemId, int quantity) {
        if (quantity <= 0) return;
        allocatedInventory[itemId] += quantity; // Adds or updates quantity for the item
    }

    // Records deallocation of an inventory item from this event
    // Returns the quantity that was actually deallocated from this event's record.
    int deallocateInventoryItem(int itemId, int quantityToDeallocate) {
        if (quantityToDeallocate <= 0) return 0;
        auto it = allocatedInventory.find(itemId);
        if (it != allocatedInventory.end()) {
            int currentQty = it->second;
            int actualDeallocated = std::min(currentQty, quantityToDeallocate);
            it->second -= actualDeallocated;
            if (it->second <= 0) {
                allocatedInventory.erase(it);
            }
            return actualDeallocated;
        }
        return 0; // Item not allocated to this event or quantity was zero
    }
    
    // Returns the string representation of the event's status
    std::string getStatusString() const {
        switch (status) {
            case EventStatus::UPCOMING: return "Upcoming";
            case EventStatus::ONGOING: return "Ongoing";
            case EventStatus::COMPLETED: return "Completed";
            case EventStatus::CANCELED: return "Canceled";
            default: return "Unknown";
        }
    }

    // Displays detailed information about the event. Requires System context to look up inventory item names.
    void displayDetails(const System& sys) const; // Definition after System class is fully defined

    // Helper to convert attendee ID list to string for file storage
    std::string attendeesToString() const {
        std::stringstream ss;
        for (size_t i = 0; i < attendeeIds.size(); ++i) {
            ss << attendeeIds[i] << (i == attendeeIds.size() - 1 ? "" : ";"); // Semicolon-separated
        }
        return ss.str();
    }

    // Helper to convert allocated inventory map to string for file storage
    std::string inventoryToString() const {
        std::stringstream ss;
        bool first = true;
        for (auto const& [itemId, quantity] : allocatedInventory) {
            if (!first) ss << ";"; // Semicolon-separated pairs
            ss << itemId << ":" << quantity; // ItemID:Quantity
            first = false;
        }
        return ss.str();
    }

    // Converts event data to a string for file storage
    std::string toString() const {
        std::stringstream ss;
        ss << eventId << "," << name << "," << date << "," << time << "," << location << ","
           << description << "," << category << "," << static_cast<int>(status) << ","
           << attendeesToString() << "," << inventoryToString();
        return ss.str();
    }

    // Static factory method to create Event objects from a string
    static Event fromString(const std::string& str) {
        std::stringstream ss(str);
        std::string segment;
        
        int id;
        std::string name, date, time, loc, desc, cat, attendeesStr, inventoryStr;
        EventStatus status;

        // Parsing each field carefully, especially the last ones
        std::getline(ss, segment, ','); id = std::stoi(segment);
        std::getline(ss, name, ',');
        std::getline(ss, date, ',');
        std::getline(ss, time, ',');
        std::getline(ss, loc, ',');
        std::getline(ss, desc, ',');
        std::getline(ss, cat, ',');
        std::getline(ss, segment, ','); status = static_cast<EventStatus>(std::stoi(segment));
        
        // Attendees and Inventory might be empty, handle carefully
        if (std::getline(ss, attendeesStr, ',')) { // if attendeesStr exists
            std::getline(ss, inventoryStr); // inventoryStr is the rest
        } else { // attendeesStr was the last field or empty
             inventoryStr = ""; // No inventory string if attendees was last or line ended
        }


        Event event(id, name, date, time, loc, desc, cat, status);

        // Parse attendees string (e.g., "1;2;3")
        if (!attendeesStr.empty()) {
            std::stringstream attSs(attendeesStr);
            std::string attIdStr;
            while (std::getline(attSs, attIdStr, ';')) {
                if(!attIdStr.empty()) event.attendeeIds.push_back(std::stoi(attIdStr));
            }
        }

        // Parse inventory string (e.g., "101:2;102:5")
        if (!inventoryStr.empty()) {
            std::stringstream invSs(inventoryStr);
            std::string itemStr;
            while (std::getline(invSs, itemStr, ';')) {
                if(!itemStr.empty()){
                    size_t colonPos = itemStr.find(':');
                    if (colonPos != std::string::npos) {
                        try {
                            int itemId = std::stoi(itemStr.substr(0, colonPos));
                            int quantity = std::stoi(itemStr.substr(colonPos + 1));
                            event.allocatedInventory[itemId] = quantity;
                        } catch (const std::exception& e) {
                            std::cerr << "Warning: Malformed inventory entry '" << itemStr << "' for event ID " << id << ". Skipping.\n";
                        }
                    }
                }
            }
        }
        return event;
    }
    static void initNextId(int id) { if (id >= nextEventId) nextEventId = id + 1;}
};
int Event::nextEventId = 1;


// ** System Class (Manages all data collections and core application logic) **
class System {
private: // Private helper method for seeding data
    void seedInitialData() {
        bool dataSeeded = false; // Flag to check if any data was seeded

        if (users.empty()) {
            std::cout << "Info: No users found in users.txt. Seeding initial user accounts.\n";
            // Add a default admin if no other admins are being seeded or for fallback
            users.push_back(new Admin("admin", "password")); 
            users.push_back(new Admin("admin2", "pass123"));
            users.push_back(new RegularUser("user1", "pass123"));
            std::cout << "Seeded user: admin (admin)\n";
            std::cout << "Seeded user: admin2 (admin)\n";
            std::cout << "Seeded user: user1 (regular)\n";
            dataSeeded = true;
        }

        if (events.empty()) {
            std::cout << "Info: No events found in events.txt. Seeding initial events.\n";
            events.emplace_back("Tech Conference 2025", "2025-10-20", "09:00", "Grand Hall", "Annual tech conference", "Conference");
            events.emplace_back("Summer Music Festival", "2025-07-15", "14:00", "City Park", "Outdoor music event", "Social");
            events.emplace_back("Art Workshop", "2025-08-05", "10:00", "Community Center", "Hands-on art workshop", "Workshop");
            std::cout << "Seeded event: Tech Conference 2025\n";
            std::cout << "Seeded event: Summer Music Festival\n";
            std::cout << "Seeded event: Art Workshop\n";
            dataSeeded = true;
        }

        if (inventory.empty()) {
            std::cout << "Info: No inventory found in inventory.txt. Seeding initial inventory items.\n";
            inventory.emplace_back("Projector", 5, "HD Projector for presentations");
            inventory.emplace_back("Microphone", 10, "Wireless microphones");
            inventory.emplace_back("Chairs", 100, "Standard event chairs");
            std::cout << "Seeded inventory: Projector (5)\n";
            std::cout << "Seeded inventory: Microphone (10)\n";
            std::cout << "Seeded inventory: Chairs (100)\n";
            dataSeeded = true;
        }
        
        if (dataSeeded) {
            std::cout << "Initial data seeded. Saving to files...\n";
            saveData(); // Save the newly seeded data to files
        }
    }

public:
    std::vector<User*> users;             // Stores all registered users (Admin, RegularUser)
    std::vector<Event> events;            // Stores all created events
    std::vector<InventoryItem> inventory; // Stores all inventory items
    std::vector<Attendee> allAttendees;   // Master list of all attendees (guests and user profiles)

    User* currentUser; // Pointer to the currently logged-in user

    // Filenames for data persistence
    const std::string USERS_FILE = "users.txt";
    const std::string EVENTS_FILE = "events.txt";
    const std::string INVENTORY_FILE = "inventory.txt";
    const std::string ATTENDEES_FILE = "attendees.txt";


    System() : currentUser(nullptr) {
        loadData(); // Load all data from files upon system startup
        // Seeding initial data is now handled in run() after loadData()
    }

    ~System() {
        saveData(); // Save all data to files upon system shutdown
        for (User* u : users) { // Clean up dynamically allocated User objects
            delete u;
        }
        users.clear();
    }

    // --- Data Loading and Saving ---
    void loadData() {
        loadUsers();
        loadEvents();
        loadInventory();
        loadAttendees(); 
        // Initialize next IDs based on loaded data
        int maxId = 0;
        for(const auto* u : users) { if(u && u->getUserId() > maxId) maxId = u->getUserId(); }
        User::initNextId(maxId +1);
        
        maxId = 0;
        for(const auto& e : events) { if(e.eventId > maxId) maxId = e.eventId; }
        Event::initNextId(maxId+1);

        maxId = 0;
        for(const auto& i : inventory) { if(i.itemId > maxId) maxId = i.itemId; }
        InventoryItem::initNextId(maxId+1);
        
        maxId = 0;
        for(const auto& a : allAttendees) { if(a.attendeeId > maxId) maxId = a.attendeeId; }
        Attendee::initNextId(maxId+1);
    }

    void saveData() {
        saveUsers();
        saveEvents();
        saveInventory();
        saveAttendees();
    }

    // User file operations
    void loadUsers(); // Definition provided after Admin and RegularUser classes are defined
    void saveUsers() {
        std::ofstream outFile(USERS_FILE);
        if (!outFile) {
            std::cerr << "Error: Could not open " << USERS_FILE << " for writing.\n";
            return;
        }
        for (const auto* user : users) {
            if (user) outFile << user->toString() << std::endl;
        }
        outFile.close();
    }

    // Event file operations
    void loadEvents() {
        std::ifstream inFile(EVENTS_FILE);
        if (!inFile) { /* std::cerr << "Info: " << EVENTS_FILE << " not found. Starting with no events.\n"; */ return; }
        std::string line;
        while (std::getline(inFile, line)) {
            if (!line.empty()) events.push_back(Event::fromString(line));
        }
        inFile.close();
    }
    void saveEvents() {
        std::ofstream outFile(EVENTS_FILE);
        if (!outFile) { std::cerr << "Error: Could not open " << EVENTS_FILE << " for writing.\n"; return; }
        for (const auto& event : events) outFile << event.toString() << std::endl;
        outFile.close();
    }

    // Inventory file operations
    void loadInventory() {
        std::ifstream inFile(INVENTORY_FILE);
        if (!inFile) { /* std::cerr << "Info: " << INVENTORY_FILE << " not found. Starting with no inventory.\n"; */ return; }
        std::string line;
        while (std::getline(inFile, line)) {
            if (!line.empty()) inventory.push_back(InventoryItem::fromString(line));
        }
        inFile.close();
    }
    void saveInventory() {
        std::ofstream outFile(INVENTORY_FILE);
        if (!outFile) { std::cerr << "Error: Could not open " << INVENTORY_FILE << " for writing.\n"; return; }
        for (const auto& item : inventory) outFile << item.toString() << std::endl;
        outFile.close();
    }

    // Attendee file operations (master list of all attendees)
    void loadAttendees() {
        std::ifstream inFile(ATTENDEES_FILE);
        if (!inFile) { /* std::cerr << "Info: " << ATTENDEES_FILE << " not found. Starting with no attendees.\n"; */ return; }
        std::string line;
        while (std::getline(inFile, line)) {
            if (!line.empty()) allAttendees.push_back(Attendee::fromString(line));
        }
        inFile.close();
    }
    void saveAttendees() {
        std::ofstream outFile(ATTENDEES_FILE);
        if (!outFile) { std::cerr << "Error: Could not open " << ATTENDEES_FILE << " for writing.\n"; return; }
        for (const auto& attendee : allAttendees) outFile << attendee.toString() << std::endl;
        outFile.close();
    }


    // --- User Management (typically called by Admin role) ---
    bool usernameExists(const std::string& username) const { // Made const
        for (const auto* user : users) {
            if (user && user->getUsername() == username) {
                return true;
            }
        }
        return false;
    }

    // Method for Admin to create user accounts
    void createUserAccount(const std::string& uname, const std::string& pwd, Role role); // Def after Admin/RegUser

    // Method for public registration
    void publicRegisterNewUser(); // Definition after Admin/RegularUser


    void deleteUserAccount(const std::string& uname) {
        auto it = std::remove_if(users.begin(), users.end(),
            [&](User* u) {
                if (u && u->getUsername() == uname) {
                    if (currentUser && currentUser->getUsername() == uname) {
                        std::cout << "Error: Cannot delete the currently logged-in user account.\n";
                        return false; // Do not remove if it's the current user
                    }
                    delete u; // Free memory of the User object
                    return true;  // Mark for removal from vector
                }
                return false;
            });

        if (it != users.end()) {
            users.erase(it, users.end());
            std::cout << "User account '" << uname << "' deleted successfully.\n";
            saveUsers();
        } else {
            std::cout << "Error: User account '" << uname << "' not found or cannot be deleted.\n";
        }
    }
    
    User* findUserByUsername(const std::string& uname) { // Non-const version
        for (auto* user : users) {
            if (user && user->getUsername() == uname) {
                return user;
            }
        }
        return nullptr;
    }
    const User* findUserByUsername(const std::string& uname) const { // Const version
        for (const auto* user : users) {
            if (user && user->getUsername() == uname) {
                return user;
            }
        }
        return nullptr;
    }
    
    void listAllUsers() const {
        std::cout << "\n--- All Users Registered in System ---\n";
        if (users.empty()) {
            std::cout << "No users currently in the system.\n";
            return;
        }
        for (const auto* user : users) {
            if (user) {
                std::cout << "ID: " << user->getUserId() << ", Username: " << user->getUsername()
                          << ", Role: " << (user->getRole() == Role::ADMIN ? "Admin" : "Regular User") << std::endl;
            }
        }
        std::cout << "-------------------------------------\n";
    }


    // --- Authentication ---
    bool login() {
        std::cout << "\n--- System Login ---\n";
        std::string uname = getStringInput("Enter username: ");
        std::string pwd = getStringInput("Enter password: ");

        for (User* u : users) {
            if (u && u->getUsername() == uname && u->getPassword() == pwd) { // Password check (unsafe in real systems)
                currentUser = u;
                std::cout << "Login successful. Welcome, " << currentUser->getUsername() 
                          << " (" << (currentUser->getRole() == Role::ADMIN ? "Admin" : "Regular User") << ")!\n";
                return true;
            }
        }
        std::cout << "Login failed: Invalid username or password.\n";
        currentUser = nullptr;
        return false;
    }

    void logout() {
        if (currentUser) {
            std::cout << "Logging out " << currentUser->getUsername() << ".\n";
            currentUser = nullptr;
        }
    }

    // --- Event Management ---
    Event* findEventById(int eventId) { // Non-const version
        for (auto& event : events) {
            if (event.eventId == eventId) return &event;
        }
        return nullptr;
    }
    const Event* findEventById(int eventId) const { // Const version
        for (const auto& event : events) {
            if (event.eventId == eventId) return &event;
        }
        return nullptr;
    }
    
    void createEvent() {
        std::cout << "\n--- Create New Event ---\n";
        std::string name = getStringInput("Enter event name: ");
        std::string date, time;
        while(true){ // Date validation loop
            date = getStringInput("Enter event date (YYYY-MM-DD): ");
            if(isValidDate(date)) break;
            std::cout << "Invalid date format. Please use YYYY-MM-DD and valid values.\n";
        }
        while(true){ // Time validation loop
            time = getStringInput("Enter event time (HH:MM, 24-hour format): ");
            if(isValidTime(time)) break;
            std::cout << "Invalid time format. Please use HH:MM (e.g., 14:30) and valid values.\n";
        }
        std::string location = getStringInput("Enter event location: ");
        std::string description = getStringInput("Enter event description: ");
        std::string category = getStringInput("Enter event category (e.g., Conference, Social, Workshop): ");

        events.emplace_back(name, date, time, location, description, category);
        std::cout << "Event '" << name << "' created successfully with ID " << events.back().eventId << ".\n";
        saveEvents(); // Persist change
    }

    void viewAllEvents(bool adminView = false) const { // `adminView` for potential future use
        std::cout << "\n--- List of All Events ---\n";
        if (events.empty()) {
            std::cout << "No events are currently scheduled in the system.\n";
            return;
        }
        for (const auto& event : events) {
            event.displayDetails(*this); // Pass system for resolving inventory item names
            std::cout << "-----------------------------------\n";
        }
    }
    
    void searchEventsByNameOrDate() const {
        std::cout << "\n--- Search Events ---\n";
        std::cout << "Search by: 1. Name Keyword 2. Exact Date (YYYY-MM-DD)\n";
        int choice = getIntInput("Enter search choice (1-2): ");
        std::string keyword;
        bool found = false;

        if (choice == 1) {
            keyword = getStringInput("Enter event name keyword to search: ");
            std::string lowerKeyword = toLower(keyword);
            std::cout << "\n--- Search Results (Name containing: '" << keyword << "') ---\n";
            for (const auto& event : events) {
                if (toLower(event.name).find(lowerKeyword) != std::string::npos) {
                    event.displayDetails(*this);
                    std::cout << "-----------------------------------\n";
                    found = true;
                }
            }
        } else if (choice == 2) {
            while(true){
                keyword = getStringInput("Enter exact event date (YYYY-MM-DD) to search: ");
                if(isValidDate(keyword)) break;
                std::cout << "Invalid date format for search. Please use YYYY-MM-DD.\n";
            }
            std::cout << "\n--- Search Results (Date: " << keyword << ") ---\n";
            for (const auto& event : events) {
                if (event.date == keyword) {
                    event.displayDetails(*this);
                    std::cout << "-----------------------------------\n";
                    found = true;
                }
            }
        } else {
            std::cout << "Invalid search choice.\n";
            return;
        }

        if (!found) {
            std::cout << "No events found matching your search criteria.\n";
        }
    }


    void editEventDetails() {
        int eventId = getIntInput("Enter ID of the event to edit: ");
        Event* event = findEventById(eventId);
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }

        std::cout << "--- Editing Event: " << event->name << " (ID: " << event->eventId << ") ---\n";
        std::cout << "Current details:\n";
        event->displayDetails(*this);
        std::cout << "-----------------------------------\n";
        std::cout << "Enter new values or press Enter to keep current.\n";

        std::string temp;
        std::cout << "New Name (" << event->name << "): "; std::getline(std::cin, temp); if (!temp.empty()) event->name = temp;
        
        std::cout << "New Date (YYYY-MM-DD) (" << event->date << "): "; std::getline(std::cin, temp); 
        if (!temp.empty()) {
            if(isValidDate(temp)) event->date = temp; else std::cout << "Invalid date format. Kept old value.\n";
        }
        
        std::cout << "New Time (HH:MM) (" << event->time << "): "; std::getline(std::cin, temp); 
        if (!temp.empty()) {
            if(isValidTime(temp)) event->time = temp; else std::cout << "Invalid time format. Kept old value.\n";
        }

        std::cout << "New Location (" << event->location << "): "; std::getline(std::cin, temp); if (!temp.empty()) event->location = temp;
        std::cout << "New Description (current: " << event->description.substr(0,20) << "...): "; std::getline(std::cin, temp); if (!temp.empty()) event->description = temp;
        std::cout << "New Category (" << event->category << "): "; std::getline(std::cin, temp); if (!temp.empty()) event->category = temp;

        std::cout << "Event details updated successfully.\n";
        saveEvents();
    }

    void deleteEvent() {
        int eventId = getIntInput("Enter ID of the event to delete: ");
        Event* eventToDel = findEventById(eventId);

        if (!eventToDel) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }

        // Deallocate all inventory associated with this event before deleting it
        for (auto const& [invId, qty] : eventToDel->allocatedInventory) {
            InventoryItem* item = findInventoryItemById(invId); // Non-const needed here
            if (item) {
                item->deallocate(qty);
            }
        }
        eventToDel->allocatedInventory.clear(); // Clear the event's own record

        // Remove the event from the main list
        auto it = std::remove_if(events.begin(), events.end(),
            [eventId](const Event& e) { return e.eventId == eventId; });
        
        if (it != events.end()) {
            events.erase(it, events.end());
            std::cout << "Event with ID " << eventId << " and its inventory allocations deleted.\n";
            
            // Optional: Clean up attendee registrations specific to this event
            // This is complex: if an Attendee object in allAttendees was *only* for this event (e.g. a guest),
            // it could be removed. If it's a user's profile, just their registration to this event is void.
            // The current model has Event.attendeeIds linking to System.allAttendees.
            // So, just removing the event is enough; the Attendee objects in allAttendees remain.
            // If an Attendee object has `eventIdRegisteredFor` matching this deleted event, that field becomes stale.
            // A more robust system might have a list of events per attendee or a joining table.
            // For now, we'll leave Attendee objects in allAttendees.
            
            saveEvents();
            saveInventory(); // Due to deallocations
        } else {
            // Should not happen if findEventById found it, but as a safeguard:
            std::cout << "Error: Event with ID " << eventId << " could not be removed (consistency issue?).\n";
        }
    }
    
    void updateEventStatus() {
        int eventId = getIntInput("Enter ID of the event to update status: ");
        Event* event = findEventById(eventId);
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }
        std::cout << "Current status of '" << event->name << "': " << event->getStatusString() << std::endl;
        std::cout << "Select new status:\n";
        std::cout << "  1. Upcoming\n  2. Ongoing\n  3. Completed\n  4. Canceled\n";
        int choice = getIntInput("Enter choice (1-4): ");
        switch (choice) {
            case 1: event->status = EventStatus::UPCOMING; break;
            case 2: event->status = EventStatus::ONGOING; break;
            case 3: event->status = EventStatus::COMPLETED; break;
            case 4: event->status = EventStatus::CANCELED; break;
            default: std::cout << "Invalid choice. Status not changed.\n"; return;
        }
        std::cout << "Event status for '" << event->name << "' updated to " << event->getStatusString() << ".\n";
        saveEvents();
    }


    // --- Attendee Management ---
    Attendee* findAttendeeInMasterList(int attendeeId) { // Non-const
        for (auto& att : allAttendees) {
            if (att.attendeeId == attendeeId) return &att;
        }
        return nullptr;
    }
    const Attendee* findAttendeeInMasterList(int attendeeId) const { // Const
        for (const auto& att : allAttendees) {
            if (att.attendeeId == attendeeId) return &att;
        }
        return nullptr;
    }

    // Registers an attendee for an event.
    // If it's a RegularUser registering themselves, their userId is used as attendeeId.
    // If Admin registers a guest, a new Attendee object is created.
    void registerAttendeeForEvent() {
        viewAllEvents();
        int eventId = getIntInput("Enter ID of the event to register for: ");
        Event* event = findEventById(eventId);
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }
        if (event->status != EventStatus::UPCOMING && event->status != EventStatus::ONGOING) {
            std::cout << "Registration is currently closed for event '" << event->name 
                      << "' (Status: " << event->getStatusString() << ").\n";
            return;
        }

        std::string name, contactInfo;
        int attendeeMasterIdToRegister = -1;

        if (currentUser && currentUser->getRole() == Role::REGULAR_USER) {
            // Regular user registering themselves
            attendeeMasterIdToRegister = currentUser->getUserId(); // Use User's ID as Attendee ID
            name = currentUser->getUsername(); // Default name from username

            // Check if an Attendee profile already exists for this user in allAttendees
            Attendee* userProfile = findAttendeeInMasterList(attendeeMasterIdToRegister);
            if (userProfile) {
                std::cout << "Registering yourself (" << name << ") using existing profile.\n";
                contactInfo = userProfile->contactInfo; // Use existing contact
                userProfile->eventIdRegisteredFor = eventId; // Update event registered for (simplistic, assumes one primary event)
            } else {
                // Create a new Attendee profile for this user in allAttendees
                std::cout << "This is your first registration. Please provide contact info.\n";
                contactInfo = getStringInput("Enter your contact info (email/phone): ");
                allAttendees.emplace_back(attendeeMasterIdToRegister, name, contactInfo, eventId, false);
                std::cout << "Profile created for " << name << ".\n";
            }
        } else {
            // Admin registering someone, or a guest (if system allows unauthenticated registration flow)
            std::cout << "--- Register New Attendee (Admin or Guest) ---\n";
            name = getStringInput("Enter attendee's full name: ");
            contactInfo = getStringInput("Enter attendee's contact info (email/phone): ");
            
            // For guests, create a new Attendee object with a new ID
            allAttendees.emplace_back(name, contactInfo, eventId); // New Attendee object with new ID
            attendeeMasterIdToRegister = allAttendees.back().attendeeId;
            std::cout << "Attendee '" << name << "' (ID: " << attendeeMasterIdToRegister << ") created.\n";
        }
        
        // Add the attendeeMasterIdToRegister to the event's list of attendees
        if (attendeeMasterIdToRegister != -1) {
            event->addAttendee(attendeeMasterIdToRegister);
            std::cout << "'" << name << "' (ID: " << attendeeMasterIdToRegister << ") successfully registered for event '" << event->name << "'.\n";
            saveEvents();
            saveAttendees(); // Master list might have changed
        } else {
            std::cout << "Error: Could not determine attendee ID for registration.\n";
        }
    }
    
    void cancelOwnRegistration() { // For RegularUser cancelling their own registration
        if (!currentUser || currentUser->getRole() != Role::REGULAR_USER) {
            std::cout << "This option is for logged-in regular users to cancel their own registration.\n";
            return;
        }

        int userAttendeeId = currentUser->getUserId(); // UserID is their AttendeeID

        std::cout << "\n--- Your Registered Events ---\n";
        std::vector<int> registeredEventIdsForUser;
        bool foundReg = false;
        for (const auto& event : events) {
            if (std::find(event.attendeeIds.begin(), event.attendeeIds.end(), userAttendeeId) != event.attendeeIds.end()) {
                std::cout << "Event ID: " << event.eventId << " - Name: " << event.name 
                          << " (" << event.getStatusString() << ")" << std::endl;
                registeredEventIdsForUser.push_back(event.eventId);
                foundReg = true;
            }
        }

        if (!foundReg) {
            std::cout << "You are not currently registered for any events.\n";
            return;
        }

        int eventIdToCancel = getIntInput("Enter ID of the event to cancel your registration from: ");
        Event* event = findEventById(eventIdToCancel);
        if (!event) {
            std::cout << "Error: Event with ID " << eventIdToCancel << " not found.\n";
            return;
        }
        if (event->status == EventStatus::COMPLETED || event->status == EventStatus::CANCELED) {
            std::cout << "Cannot cancel registration for an event that is already " << event->getStatusString() << ".\n";
            return;
        }
        
        // Check if user is actually registered for this specific event
        auto it_user_in_event = std::find(event->attendeeIds.begin(), event->attendeeIds.end(), userAttendeeId);
        if (it_user_in_event != event->attendeeIds.end()) {
            event->removeAttendee(userAttendeeId); // Remove from event's list
            
            // Optional: Update the Attendee object in allAttendees if eventIdRegisteredFor was specific
            Attendee* userProfile = findAttendeeInMasterList(userAttendeeId);
            if (userProfile && userProfile->eventIdRegisteredFor == eventIdToCancel) {
                userProfile->eventIdRegisteredFor = 0; // Mark as not primarily for this event anymore
                userProfile->isCheckedIn = false; // Reset check-in status if any
            }
            std::cout << "Your registration for event '" << event->name << "' has been cancelled.\n";
            saveEvents();
            saveAttendees();
        } else {
            std::cout << "You do not seem to be registered for event '" << event->name << "'.\n";
        }
    }


    void viewAttendeeListsPerEvent() const {
        viewAllEvents();
        int eventId = getIntInput("Enter ID of the event to view its attendee list: ");
        const Event* event = findEventById(eventId); // Use const version
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }

        std::cout << "\n--- Attendee List for Event: " << event->name << " (ID: " << event->eventId << ") ---\n";
        if (event->attendeeIds.empty()) {
            std::cout << "No attendees are currently registered for this event.\n";
            return;
        }

        for (int attendeeIdFromEvent : event->attendeeIds) {
            const Attendee* attProfile = findAttendeeInMasterList(attendeeIdFromEvent); // Const version
            if (attProfile) {
                attProfile->displayDetails();
            } else {
                // This might happen if an ID in event.attendeeIds doesn't map to allAttendees.
                // Could be a data consistency issue or an ID of a user not yet in allAttendees as a profile.
                std::cout << "Attendee with ID " << attendeeIdFromEvent 
                          << " listed in event, but profile not found in master list (data inconsistency?).\n";
            }
        }
        std::cout << "----------------------------------------------------------\n";
    }

    void checkInAttendeeForEvent() {
        viewAllEvents();
        int eventId = getIntInput("Enter ID of the event for attendee check-in: ");
        Event* event = findEventById(eventId); // Non-const needed if event status might change implicitly
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }
        if (event->status == EventStatus::COMPLETED || event->status == EventStatus::CANCELED) {
            std::cout << "Cannot perform check-in for an event that is already " << event->getStatusString() << ".\n";
            return;
        }
        if (event->status == EventStatus::UPCOMING) {
            char confirmOngoing;
            std::cout << "Event '" << event->name << "' is still 'Upcoming'. Do you want to mark it as 'Ongoing' to proceed with check-in? (y/n): ";
            std::cin >> confirmOngoing;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
            if (tolower(confirmOngoing) == 'y') {
                event->status = EventStatus::ONGOING;
                saveEvents();
                std::cout << "Event status updated to 'Ongoing'.\n";
            } else {
                std::cout << "Check-in aborted. Event status not changed.\n";
                return;
            }
        }


        std::cout << "\n--- Registered Attendees for Event: " << event->name << " ---\n";
        if (event->attendeeIds.empty()) {
            std::cout << "No attendees registered for this event to check-in.\n";
            return;
        }
        for (int attId : event->attendeeIds) { // Display attendees of this event
             const Attendee* masterAtt = findAttendeeInMasterList(attId);
             if(masterAtt) masterAtt->displayDetails();
        }
        std::cout << "---------------------------------------------------\n";

        int attendeeIdToCheckIn = getIntInput("Enter Attendee ID to check-in: ");
        
        // Verify this attendee ID is actually registered for THIS event
        bool isRegisteredForThisEvent = false;
        for(int idInEvent : event->attendeeIds){
            if(idInEvent == attendeeIdToCheckIn){
                isRegisteredForThisEvent = true;
                break;
            }
        }
        if(!isRegisteredForThisEvent){
            std::cout << "Error: Attendee with ID " << attendeeIdToCheckIn 
                      << " is not registered for event '" << event->name << "'.\n";
            return;
        }

        Attendee* attendeeToModify = findAttendeeInMasterList(attendeeIdToCheckIn); // Non-const needed
        if (attendeeToModify) {
            attendeeToModify->checkIn(); // This method handles if already checked in
            saveAttendees(); // Persist the check-in status change in the master list
        } else {
            std::cout << "Error: Attendee profile with ID " << attendeeIdToCheckIn 
                      << " not found in master records. Cannot perform check-in.\n";
        }
    }
    
    void generateAttendanceReportForEvent() const {
        viewAllEvents();
        int eventId = getIntInput("Enter ID of the event for its attendance report: ");
        const Event* event = findEventById(eventId);
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }

        std::cout << "\n--- Attendance Report for Event: " << event->name << " (ID: " << event->eventId << ") ---\n";
        int totalRegistered = event->attendeeIds.size();
        int totalCheckedIn = 0;

        if (totalRegistered == 0) {
            std::cout << "No attendees were registered for this event.\n";
        } else {
            std::cout << "Registered Attendees (" << totalRegistered << "):\n";
            for (int attendeeId : event->attendeeIds) {
                const Attendee* attProfile = findAttendeeInMasterList(attendeeId);
                if (attProfile) {
                    std::cout << "  ID: " << attProfile->attendeeId << ", Name: " << attProfile->name 
                              << ", Checked-in: " << (attProfile->isCheckedIn ? "Yes" : "No") << std::endl;
                    if (attProfile->isCheckedIn) {
                        totalCheckedIn++;
                    }
                } else {
                    std::cout << "  ID: " << attendeeId << " (Profile not found in master list)\n";
                }
            }
            std::cout << "\nSummary:\n";
            std::cout << "Total Registered: " << totalRegistered << std::endl;
            std::cout << "Total Checked-In: " << totalCheckedIn << std::endl;
        }
        std::cout << "-------------------------------------------------------\n";
    }

    void exportAttendeeListForEventToFile() const {
        viewAllEvents();
        int eventId = getIntInput("Enter Event ID to export its attendee list: ");
        const Event* event = findEventById(eventId);
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found for export.\n";
            return;
        }
        std::string filename = "attendees_event_" + std::to_string(eventId) + ".txt";
        filename = getStringInput("Enter filename for export (default: " + filename + "): ");
        if (filename.empty()) filename = "attendees_event_" + std::to_string(eventId) + ".txt";


        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error: Could not open file '" << filename << "' for writing.\n";
            return;
        }

        outFile << "Attendee List for Event: " << event->name << " (ID: " << event->eventId << ")\n";
        outFile << "Status: " << event->getStatusString() << "\n";
        outFile << "Date: " << event->date << ", Time: " << event->time << ", Location: " << event->location << "\n";
        outFile << "--------------------------------------------------\n";
        outFile << "AttendeeID,Name,ContactInfo,CheckedInStatus\n";
        
        if (event->attendeeIds.empty()) {
            outFile << "No attendees registered for this event.\n";
        } else {
            for (int attendeeId : event->attendeeIds) {
                const Attendee* attProfile = findAttendeeInMasterList(attendeeId);
                if (attProfile) {
                    outFile << attProfile->attendeeId << "," << attProfile->name << "," 
                            << attProfile->contactInfo << "," << (attProfile->isCheckedIn ? "Yes" : "No") << "\n";
                } else {
                     outFile << attendeeId << ",(Profile Not Found),N/A,N/A\n";
                }
            }
        }
        outFile.close();
        std::cout << "Attendee list for event '" << event->name << "' exported to '" << filename << "' successfully.\n";
    }


    // --- Inventory Management ---
    InventoryItem* findInventoryItemById(int itemId) { // Non-const
        for (auto& item : inventory) {
            if (item.itemId == itemId) return &item;
        }
        return nullptr;
    }
    const InventoryItem* findInventoryItemById(int itemId) const { // Const
        for (const auto& item : inventory) {
            if (item.itemId == itemId) return &item;
        }
        return nullptr;
    }
    
    InventoryItem* findInventoryItemByName(const std::string& name) { // Non-const
        std::string lowerName = toLower(name);
        for (auto& item : inventory) {
            if (toLower(item.name) == lowerName) return &item;
        }
        return nullptr;
    }
    const InventoryItem* findInventoryItemByName(const std::string& name) const { // Const
        std::string lowerName = toLower(name);
        for (const auto& item : inventory) {
            if (toLower(item.name) == lowerName) return &item;
        }
        return nullptr;
    }


    void addInventoryItem() {
        std::cout << "\n--- Add New Inventory Item ---\n";
        std::string name = getStringInput("Enter item name: ");
        if(findInventoryItemByName(name)){ // Check for name uniqueness
            std::cout << "Error: An inventory item with the name '" << name << "' already exists. Item names must be unique.\n";
            return;
        }
        int quantity = getPositiveIntInput("Enter total quantity of this item: ");
        std::string description = getStringInput("Enter item description: ");

        inventory.emplace_back(name, quantity, description);
        std::cout << "Inventory item '" << name << "' added successfully with ID " << inventory.back().itemId << ".\n";
        saveInventory();
    }
    
    void updateInventoryItemDetails() {
        viewAllInventoryItems();
        int itemId = getIntInput("Enter ID of the inventory item to update: ");
        InventoryItem* item = findInventoryItemById(itemId); // Non-const needed
        if(!item) {
            std::cout << "Error: Inventory item with ID " << itemId << " not found.\n";
            return;
        }
        std::cout << "--- Updating Item: " << item->name << " (ID: " << item->itemId << ") ---\n";
        std::cout << "Current details: "; item->displayDetails();
        std::cout << "Enter new values or press Enter to keep current.\n";

        std::string temp;
        std::cout << "New Name (" << item->name << "): "; std::getline(std::cin, temp); 
        if (!temp.empty()) {
            const InventoryItem* existingByName = findInventoryItemByName(temp); // Check new name uniqueness
            if(existingByName && existingByName->itemId != item->itemId) { // If name exists and it's not the current item
                std::cout << "Error: Another inventory item with the name '" << temp << "' already exists. Kept old name.\n";
            } else {
                item->name = temp;
            }
        }

        std::cout << "New Total Quantity (current: " << item->totalQuantity << ", allocated: " << item->allocatedQuantity << "): "; 
        std::string qtyStr;
        std::getline(std::cin, qtyStr);
        if(!qtyStr.empty()){
            try {
                int newQty = std::stoi(qtyStr);
                item->setTotalQuantity(newQty); // This method has validation
            } catch (const std::invalid_argument&) {
                std::cout << "Invalid quantity input. Total quantity not changed.\n";
            } catch (const std::out_of_range&) {
                 std::cout << "Quantity input out of range. Total quantity not changed.\n";
            }
        }
        
        std::cout << "New Description (current: " << item->description.substr(0,30) << "...): "; std::getline(std::cin, temp); 
        if (!temp.empty()) item->description = temp;

        std::cout << "Inventory item details updated successfully.\n";
        saveInventory();
    }


    void viewAllInventoryItems() const {
        std::cout << "\n--- Complete Inventory List ---\n";
        if (inventory.empty()) {
            std::cout << "No items currently in inventory.\n";
            return;
        }
        for (const auto& item : inventory) {
            item.displayDetails();
        }
        std::cout << "-------------------------------\n";
    }

    void trackInventoryAllocationToEvent() {
        viewAllEvents();
        int eventId = getIntInput("Enter ID of the event to allocate/deallocate inventory for: ");
        Event* event = findEventById(eventId); // Non-const needed
        if (!event) {
            std::cout << "Error: Event with ID " << eventId << " not found.\n";
            return;
        }
        if (event->status == EventStatus::COMPLETED || event->status == EventStatus::CANCELED) {
            std::cout << "Cannot modify inventory for an event that is " << event->getStatusString() << ".\n";
            return;
        }


        viewAllInventoryItems();
        int itemId = getIntInput("Enter ID of the inventory item to manage for this event: ");
        InventoryItem* item = findInventoryItemById(itemId); // Non-const needed
        if (!item) {
            std::cout << "Error: Inventory item with ID " << itemId << " not found.\n";
            return;
        }

        std::cout << "\nManaging inventory for Event: '" << event->name << "' (ID: " << event->eventId << ")\n";
        std::cout << "Item: '" << item->name << "' (ID: " << item->itemId << ")\n";
        std::cout << "  Item Total Quantity: " << item->totalQuantity 
                  << ", Item Currently Allocated (All Events): " << item->allocatedQuantity
                  << ", Item Available Overall: " << item->getAvailableQuantity() << std::endl;
        int currentlyAllocatedToThisEvent = event->allocatedInventory.count(itemId) ? event->allocatedInventory.at(itemId) : 0;
        std::cout << "  Currently Allocated to THIS Event: " << currentlyAllocatedToThisEvent << std::endl;

        std::cout << "\nOptions:\n  1. Allocate more to this event\n  2. Deallocate from this event\n  3. Cancel\n";
        int choice = getIntInput("Enter choice (1-3): ");

        if (choice == 1) { // Allocate
            int quantityToAllocate = getPositiveIntInput("Enter quantity of '" + item->name + "' to ALLOCATE to this event: ");
            if (item->allocate(quantityToAllocate)) { // Tries to reserve from item's general pool
                event->allocateInventoryItem(itemId, quantityToAllocate); // Records it in the event's map
                std::cout << quantityToAllocate << " of '" << item->name << "' successfully allocated to event '" << event->name << "'.\n";
                saveInventory();
                saveEvents();
            } else {
                std::cout << "Failed to allocate. Not enough available or invalid quantity.\n";
            }
        } else if (choice == 2) { // Deallocate
            if (currentlyAllocatedToThisEvent == 0) {
                std::cout << "No units of '" << item->name << "' are currently allocated to this event to deallocate.\n";
                return;
            }
            int quantityToDeallocate = getPositiveIntInput("Enter quantity of '" + item->name + "' to DEALLOCATE from this event (max " + std::to_string(currentlyAllocatedToThisEvent) + "): ");
            
            int actualDeallocatedFromEvent = event->deallocateInventoryItem(itemId, quantityToDeallocate);
            if (actualDeallocatedFromEvent > 0) {
                item->deallocate(actualDeallocatedFromEvent); // Returns it to item's general pool
                std::cout << actualDeallocatedFromEvent << " of '" << item->name << "' successfully deallocated from event '" << event->name << "'.\n";
                saveInventory();
                saveEvents();
            } else {
                 std::cout << "Deallocation failed or no units were specified/available to deallocate from this event.\n";
            }
        } else {
            std::cout << "Inventory allocation cancelled.\n";
        }
    }
    
    void generateFullInventoryReport() const {
        std::cout << "\n--- Comprehensive Inventory Report ---\n";
        if (inventory.empty()) {
            std::cout << "No items currently in inventory.\n";
            return;
        }
        std::cout << "=== Overall Inventory Status ===\n";
        std::cout << "ID\tName\t\tTotal\tAllocated (All)\tAvailable\tDescription\n";
        std::cout << "--------------------------------------------------------------------------------------\n";
        for (const auto& item : inventory) {
            std::cout << item.itemId << "\t" << item.name << (item.name.length() < 8 ? "\t\t" : "\t")
                      << item.totalQuantity << "\t" << item.allocatedQuantity << "\t\t"
                      << item.getAvailableQuantity() << "\t\t" << item.description << std::endl;
        }
        std::cout << "--------------------------------------------------------------------------------------\n";

        std::cout << "\n=== Inventory Allocation by Event ===\n";
        bool anyAllocations = false;
        if(events.empty()){
            std::cout << "No events exist to show allocations for.\n";
        } else {
            for(const auto& event : events){
                if(!event.allocatedInventory.empty()){
                    anyAllocations = true;
                    std::cout << "Event: '" << event.name << "' (ID: " << event.eventId 
                              << ", Status: " << event.getStatusString() << ")\n";
                    for(auto const& [itemId, qty] : event.allocatedInventory){
                        const InventoryItem* item = findInventoryItemById(itemId);
                        std::cout << "  - Item: " << (item ? item->name : "Unknown Item") 
                                  << " (ID: " << itemId << "), Quantity Allocated: " << qty << std::endl;
                    }
                    std::cout << "  ---\n";
                }
            }
            if (!anyAllocations && !events.empty()) {
                 std::cout << "No inventory is currently allocated to any events.\n";
            }
        }
         std::cout << "--------------------------------------------------------------------------------------\n";
    }


    // --- Data Export (Admin Feature) ---
    void exportAllEventsDataToFile() const {
        std::string filename = "all_events_export.txt";
        filename = getStringInput("Enter filename for events export (default: " + filename + "): ");
        if (filename.empty()) filename = "all_events_export.txt";

        std::ofstream outFile(filename);
        if (!outFile) { std::cerr << "Error: Could not open '" << filename << "' for writing.\n"; return; }
        
        outFile << "EventID,Name,Date,Time,Location,Description,Category,Status(Numeric),AttendeeIDs(SemicolonSep),AllocatedInventory(ItemID:Qty;SemicolonSep)\n";
        for (const auto& event : events) {
            outFile << event.toString() << std::endl;
        }
        outFile.close();
        std::cout << "All events data exported to '" << filename << "' successfully.\n";
    }

    void exportAllAttendeesDataToFile() const { // Exports the master attendee list from System.allAttendees
        std::string filename = "all_attendees_master_list_export.txt";
        filename = getStringInput("Enter filename for master attendees export (default: " + filename + "): ");
        if (filename.empty()) filename = "all_attendees_master_list_export.txt";

        std::ofstream outFile(filename);
        if (!outFile) { std::cerr << "Error: Could not open '" << filename << "' for writing.\n"; return; }

        outFile << "AttendeeID,Name,ContactInfo,PrimaryEventIDRegisteredFor,CheckedInStatus(1=Yes;0=No)\n";
        for (const auto& attendee : allAttendees) {
            outFile << attendee.toString() << std::endl;
        }
        outFile.close();
        std::cout << "Master list of all attendees data exported to '" << filename << "' successfully.\n";
    }

    void exportAllInventoryDataToFile() const {
        std::string filename = "all_inventory_export.txt";
        filename = getStringInput("Enter filename for inventory export (default: " + filename + "): ");
        if (filename.empty()) filename = "all_inventory_export.txt";

        std::ofstream outFile(filename);
        if (!outFile) { std::cerr << "Error: Could not open '" << filename << "' for writing.\n"; return; }

        outFile << "ItemID,Name,TotalQuantity,AllocatedQuantity(AllEvents),Description\n";
        for (const auto& item : inventory) {
            outFile << item.toString() << std::endl;
        }
        outFile.close();
        std::cout << "All inventory data exported to '" << filename << "' successfully.\n";
    }

    // --- Main Application Loop ---
    void run(); // Declaration moved here, definition will be after Admin/RegularUser
    
    // Method for a RegularUser to update their own contact information
    // This updates their corresponding Attendee profile in `allAttendees`.
    void updateCurrentLoggedInUserContactInfo() {
        if (!currentUser || currentUser->getRole() != Role::REGULAR_USER) {
             std::cout << "Error: This function is for logged-in regular users only.\n"; return;
        }

        int userAttendeeId = currentUser->getUserId(); // User's ID is their Attendee ID
        Attendee* userProfile = findAttendeeInMasterList(userAttendeeId);

        if (userProfile) {
            std::cout << "Current contact info: " << userProfile->contactInfo << std::endl;
            std::string newContact = getStringInput("Enter new contact info (email/phone): ");
            userProfile->contactInfo = newContact;
            std::cout << "Your contact information has been updated successfully.\n";
        } else {
            // This case implies the user logged in but doesn't have an Attendee profile yet.
            // This might happen if they haven't registered for an event or updated info before.
            // We should create one for them.
            std::cout << "No contact profile found. Let's create one.\n";
            std::string newContact = getStringInput("Enter your contact info (email/phone): ");
            allAttendees.emplace_back(userAttendeeId, currentUser->getUsername(), newContact, 0, false); // 0 for no specific event yet
            std::cout << "Your contact profile has been created and updated.\n";
        }
        saveAttendees(); // Persist changes to the master attendee list
    }
};


// --- Derived User Classes (Admin, RegularUser) ---
// These need to be defined *after* System class if their methods heavily use System members,
// or forward declare System and define methods later. Here, methods are defined inline.

// ** Admin Class **
// Derived from User, representing an administrator with full system control.
class Admin : public User {
public:
    // Constructor for new Admin
    Admin(std::string uname, std::string pwd) : User(std::move(uname), std::move(pwd), Role::ADMIN) {}
    // Constructor for loading Admin from file
    Admin(int id, std::string uname, std::string pwd) : User(id, std::move(uname), std::move(pwd), Role::ADMIN) {}


    // Overrides User's pure virtual displayMenu to show Admin-specific options.
    void displayMenu(System& sys) override {
        int choice;
        while (sys.currentUser == this) { // Loop as long as this admin is logged in
            std::cout << "\n--- Admin Menu (Logged in as: " << username << ") ---\n";
            std::cout << "1. User Account Management\n";
            std::cout << "2. Event Management\n";
            std::cout << "3. Attendee Management (Admin)\n";
            std::cout << "4. Inventory Management\n";
            std::cout << "5. Data Export Utilities\n";
            std::cout << "6. Logout\n";
            choice = getIntInput("Enter your choice (1-6): ");

            switch (choice) {
                case 1: adminUserManagementMenu(sys); break;
                case 2: adminEventManagementMenu(sys); break;
                case 3: adminAttendeeManagementMenu(sys); break;
                case 4: adminInventoryManagementMenu(sys); break;
                case 5: adminDataExportMenu(sys); break;
                case 6: sys.logout(); return; // Exit this menu and trigger logout in System
                default: std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }

private:
    // Sub-menu for managing user accounts
    void adminUserManagementMenu(System& sys) {
        int choice;
        while (sys.currentUser == this) {
            std::cout << "\n  -- User Account Management (Admin) --\n";
            std::cout << "  1. Create New User Account (Admin/Regular)\n";
            std::cout << "  2. Delete User Account\n";
            std::cout << "  3. List All User Accounts\n";
            // Future: Update user account (e.g., reset password, change role)
            std::cout << "  4. Back to Admin Main Menu\n";
            choice = getIntInput("  Enter choice (1-4): ");
            std::string uname, pwd;
            int roleChoiceNum;

            switch (choice) {
                case 1:
                    uname = getStringInput("  Enter username for the new user: ");
                    if (sys.usernameExists(uname)) {
                        std::cout << "  Error: Username '" << uname << "' already exists. Cannot create user.\n";
                        break;
                    }
                    pwd = getStringInput("  Enter temporary password for new user (min 6 chars): ");
                    if (pwd.length() < 6) {
                         std::cout << "  Password too short. User not created.\n"; break;
                    }
                    std::cout << "  Select role for new user: 1. Admin  2. Regular User\n";
                    roleChoiceNum = getIntInput("  Enter role choice (1-2): ");
                    sys.createUserAccount(uname, pwd, (roleChoiceNum == 1 ? Role::ADMIN : Role::REGULAR_USER));
                    break;
                case 2:
                    uname = getStringInput("  Enter username of the account to delete: ");
                    if (sys.currentUser && uname == sys.currentUser->getUsername()){ // Check if sys.currentUser is not null
                        std::cout << "  Error: You cannot delete your own currently active account.\n"; break;
                    }
                    sys.deleteUserAccount(uname);
                    break;
                case 3:
                    sys.listAllUsers();
                    break;
                case 4: return; // Go back to main admin menu
                default: std::cout << "  Invalid choice in user management.\n";
            }
        }
    }

    // Sub-menu for managing events
    void adminEventManagementMenu(System& sys) {
        int choice;
        while (sys.currentUser == this) {
            std::cout << "\n  -- Event Management (Admin) --\n";
            std::cout << "  1. Create New Event\n";
            std::cout << "  2. View All Events (Detailed)\n";
            std::cout << "  3. Search Events (by Name/Date)\n";
            std::cout << "  4. Edit Event Details\n";
            std::cout << "  5. Delete Event\n";
            std::cout << "  6. Update Event Status (Upcoming/Ongoing/Completed/Canceled)\n";
            std::cout << "  7. Back to Admin Main Menu\n";
            choice = getIntInput("  Enter choice (1-7): ");
            switch (choice) {
                case 1: sys.createEvent(); break;
                case 2: sys.viewAllEvents(true); break; // true for potential admin-specific view details
                case 3: sys.searchEventsByNameOrDate(); break;
                case 4: sys.editEventDetails(); break;
                case 5: sys.deleteEvent(); break;
                case 6: sys.updateEventStatus(); break;
                case 7: return;
                default: std::cout << "  Invalid choice in event management.\n";
            }
        }
    }
    
    // Sub-menu for managing attendees (Admin capabilities)
    void adminAttendeeManagementMenu(System& sys) {
        int choice;
        while(sys.currentUser == this) {
            std::cout << "\n  -- Attendee Management (Admin) --\n";
            std::cout << "  1. Register an Attendee for an Event (Admin a/c)\n"; 
            std::cout << "  2. View Attendee Lists (Per Event)\n";
            std::cout << "  3. Perform Attendee Check-in for an Event\n";
            std::cout << "  4. Generate Attendance Report for an Event\n";
            std::cout << "  5. Export Attendee List for a Specific Event to File\n";
            std::cout << "  6. Back to Admin Main Menu\n";
            choice = getIntInput("  Enter choice (1-6): ");
            switch (choice) {
                case 1: sys.registerAttendeeForEvent(); break; // Admin uses the general registration, can input any name/contact
                case 2: sys.viewAttendeeListsPerEvent(); break;
                case 3: sys.checkInAttendeeForEvent(); break;
                case 4: sys.generateAttendanceReportForEvent(); break;
                case 5: sys.exportAttendeeListForEventToFile(); break;
                case 6: return;
                default: std::cout << "  Invalid choice in attendee management.\n";
            }
        }
    }

    // Sub-menu for managing inventory
    void adminInventoryManagementMenu(System& sys) {
        int choice;
        while(sys.currentUser == this) {
            std::cout << "\n  -- Inventory Management (Admin) --\n";
            std::cout << "  1. Add New Item to Inventory\n";
            std::cout << "  2. View All Inventory Items\n";
            std::cout << "  3. Update Inventory Item Details (Name, Total Qty, Desc)\n";
            std::cout << "  4. Allocate/Deallocate Inventory for a Specific Event\n";
            std::cout << "  5. Generate Full Inventory Report (Overall & Per Event)\n";
            std::cout << "  6. Back to Admin Main Menu\n";
            choice = getIntInput("  Enter choice (1-6): ");
            switch (choice) {
                case 1: sys.addInventoryItem(); break;
                case 2: sys.viewAllInventoryItems(); break;
                case 3: sys.updateInventoryItemDetails(); break;
                case 4: sys.trackInventoryAllocationToEvent(); break;
                case 5: sys.generateFullInventoryReport(); break;
                case 6: return;
                default: std::cout << "  Invalid choice in inventory management.\n";
            }
        }
    }
    
    // Sub-menu for data export utilities
    void adminDataExportMenu(System& sys) {
        int choice;
        while(sys.currentUser == this) {
            std::cout << "\n  -- Data Export Utilities (Admin) --\n";
            std::cout << "  1. Export All Events Data to File\n";
            std::cout << "  2. Export Master List of All Attendees to File\n";
            std::cout << "  3. Export All Inventory Data to File\n";
            std::cout << "  4. Back to Admin Main Menu\n";
            choice = getIntInput("  Enter choice (1-4): ");
            switch (choice) {
                case 1: sys.exportAllEventsDataToFile(); break;
                case 2: sys.exportAllAttendeesDataToFile(); break;
                case 3: sys.exportAllInventoryDataToFile(); break;
                case 4: return;
                default: std::cout << "  Invalid choice in data export.\n";
            }
        }
    }
};

// ** RegularUser Class **
// Derived from User, representing a standard user (attendee/guest) with limited system access.
class RegularUser : public User {
public:
    // Constructor for new RegularUser
    RegularUser(std::string uname, std::string pwd) : User(std::move(uname), std::move(pwd), Role::REGULAR_USER) {}
    // Constructor for loading RegularUser from file
    RegularUser(int id, std::string uname, std::string pwd) : User(id, std::move(uname), std::move(pwd), Role::REGULAR_USER) {}

    // Overrides User's pure virtual displayMenu to show RegularUser-specific options.
    void displayMenu(System& sys) override {
        int choice;
        while (sys.currentUser == this) { // Loop as long as this user is logged in
            std::cout << "\n--- Regular User Menu (Logged in as: " << username << ") ---\n";
            std::cout << "1. Browse All Available Events\n";
            std::cout << "2. Search Events (by Name Keyword or Date)\n";
            std::cout << "3. Register Myself for an Event\n";
            std::cout << "4. Cancel My Registration for an Event\n";
            std::cout << "5. View Attendee List for a Specific Event (Read-Only)\n";
            std::cout << "6. Update My Contact Information (for event communication)\n";
            std::cout << "7. Change My Password\n";
            std::cout << "8. Logout\n";
            choice = getIntInput("Enter your choice (1-8): ");

            switch (choice) {
                case 1: sys.viewAllEvents(); break;
                case 2: sys.searchEventsByNameOrDate(); break;
                case 3: sys.registerAttendeeForEvent(); break; // System context will know it's this user
                case 4: sys.cancelOwnRegistration(); break; // System context for current user
                case 5: sys.viewAttendeeListsPerEvent(); break; // Read-only access
                case 6: 
                    sys.updateCurrentLoggedInUserContactInfo();
                    break;
                case 7: {
                    std::cout << "--- Change Password ---\n";
                    std::string currentPass = getStringInput("Enter your CURRENT password for verification: ");
                    if (currentPass != password) { // `password` is the User base class member
                        std::cout << "Incorrect current password. Password change aborted.\n";
                        break;
                    }
                    std::string newPass = getStringInput("Enter new password (min 6 characters): ");
                    std::string confirmPass = getStringInput("Confirm new password: ");
                    if (newPass != confirmPass) {
                        std::cout << "New passwords do not match. Password change aborted.\n";
                        break;
                    }
                    setPassword(newPass); // User class method for setting password (includes length validation)
                    if (sys.currentUser) { // Ensure currentUser is not null before saving
                         sys.saveUsers();      // Persist the password change to file
                    } else {
                        std::cout << "Error: Cannot save password change. User session is invalid.\n";
                    }
                    break;
                }
                case 8: sys.logout(); return; // Exit this menu and trigger logout in System
                default: std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }
};

// --- User Factory Method Definition (User::fromString) ---
// This needs to be defined after Admin and RegularUser classes are fully defined.
User* User::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string segment;
    int id;
    std::string uname, pwd;
    Role role_val;

    // Basic validation: check if line is empty or too short
    if (str.empty() || std::count(str.begin(), str.end(), ',') < 3) {
        std::cerr << "Warning: Malformed user data line: '" << str << "'. Skipping.\n";
        return nullptr;
    }

    try {
        std::getline(ss, segment, ','); id = std::stoi(segment);
        std::getline(ss, uname, ',');
        std::getline(ss, pwd, ','); // Password stored as plaintext (BAD for real systems)
        std::getline(ss, segment, ','); role_val = static_cast<Role>(std::stoi(segment));
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Warning: Invalid data format in user line (stoi failed): '" << str << "'. " << ia.what() << ". Skipping.\n";
        return nullptr;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Warning: Data out of range in user line (stoi failed): '" << str << "'. " << oor.what() << ". Skipping.\n";
        return nullptr;
    }


    if (role_val == Role::ADMIN) {
        return new Admin(id, uname, pwd);
    } else if (role_val == Role::REGULAR_USER) {
        return new RegularUser(id, uname, pwd);
    }
    std::cerr << "Warning: Unknown role in user data line: '" << str << "'. Skipping.\n";
    return nullptr; // Should not happen if data is valid and Role enum is handled
}

// --- System Method Definitions (moved after dependent class definitions) ---

// --- System::loadUsers Definition ---
void System::loadUsers() {
    std::ifstream inFile(USERS_FILE);
    if (!inFile) {
        // std::cerr << "Info: " << USERS_FILE << " not found. System will start without pre-loaded users.\n";
        // (Default admin will be created by System::run if users vector remains empty)
        return;
    }
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            User* user = User::fromString(line); // User::fromString handles Admin/RegularUser creation
            if (user) {
                users.push_back(user);
            }
        }
    }
    inFile.close();
}

// --- System::createUserAccount Definition (Called by Admin or public registration) ---
void System::createUserAccount(const std::string& uname, const std::string& pwd, Role role) {
    // Username existence check should be done by the caller before calling this.
    // Password validation (e.g. length) should also be done by the caller.
    if (role == Role::ADMIN) {
        users.push_back(new Admin(uname, pwd)); // Uses User constructor that increments nextUserId
    } else if (role == Role::REGULAR_USER) {
        users.push_back(new RegularUser(uname, pwd));
    } else {
        std::cout << "Error: Invalid role specified during user creation.\n";
        return;
    }
    std::cout << (role == Role::ADMIN ? "Admin" : "Regular User") 
              << " account '" << uname << "' created successfully with ID " << users.back()->getUserId() << ".\n";
    saveUsers(); // Persist change immediately
}

// --- System::publicRegisterNewUser Definition ---
void System::publicRegisterNewUser() {
    std::cout << "\n--- Register New User Account ---\n";
    std::string uname = getStringInput("Enter desired username: ");
    if (usernameExists(uname)) {
        std::cout << "Error: Username '" << uname << "' already exists. Please choose a different username.\n";
        return;
    }

    std::string pwd = getStringInput("Enter password (min 6 characters): ");
    if (pwd.length() < 6) {
        std::cout << "Password is too short (minimum 6 characters required). Account not created.\n";
        return;
    }
    // Optional: Add password confirmation step here
    // std::string confirmPwd = getStringInput("Confirm password: ");
    // if (pwd != confirmPwd) {
    //     std::cout << "Passwords do not match. Account not created.\n";
    //     return;
    // }

    std::cout << "Select account type:\n";
    std::cout << "  1. Administrator Account\n";
    std::cout << "  2. Regular User Account\n";
    int roleChoiceNum = getIntInput("Enter choice (1-2): ");

    Role selectedRole;
    if (roleChoiceNum == 1) {
        selectedRole = Role::ADMIN;
    } else if (roleChoiceNum == 2) {
        selectedRole = Role::REGULAR_USER;
    } else {
        std::cout << "Invalid role selection. Account not created.\n";
        return;
    }

    // Call the existing createUserAccount method which handles adding to vector and saving
    createUserAccount(uname, pwd, selectedRole);
}


// --- System::run Definition ---
void System::run() {
    loadData(); // Load data first
    seedInitialData(); // Then, seed initial data if collections are empty

    while (true) {
        if (!currentUser) { // If no user is logged in, show login/exit menu
            std::cout << "\n===== Event Management System Main Menu =====\n";
            std::cout << "1. Login to System\n";
            std::cout << "2. Register New Account\n"; 
            std::cout << "3. Exit System\n";
            int choice = getIntInput("Enter your choice (1-3): ");
            switch (choice) {
                case 1:
                    login(); // Attempt login
                    break;
                case 2:
                    publicRegisterNewUser(); // Call the new public registration method
                    break;
                case 3:
                    std::cout << "Exiting Event Management System. Goodbye!\n";
                    return; // Exit the run method, which will lead to main returning
                default:
                    std::cout << "Invalid choice. Please enter 1, 2, or 3.\n";
            }
        } else {
            // If a user is logged in, display their role-specific menu
            currentUser->displayMenu(*this); // Pass reference to the System object
        }
    }
}


// --- Event::displayDetails Definition (requires System to be defined) ---
void Event::displayDetails(const System& sys) const {
    std::cout << "Event ID: " << eventId << "\n"
              << "  Name: " << name << "\n"
              << "  Date: " << date << ", Time: " << time << "\n"
              << "  Location: " << location << "\n"
              << "  Category: " << category << "\n"
              << "  Status: " << getStatusString() << "\n"
              << "  Description: " << description << "\n"
              << "  Registered Attendees Count: " << attendeeIds.size() << "\n";
    if (!allocatedInventory.empty()) {
        std::cout << "  Allocated Inventory Items:\n";
        for (auto const& [invId, quantity] : allocatedInventory) {
            const InventoryItem* item = sys.findInventoryItemById(invId); // Uses const version of find
            std::cout << "    - Item: " << (item ? item->name : "Unknown Item ID " + std::to_string(invId))
                      << " (ID: " << invId << "), Quantity: " << quantity << "\n";
        }
    } else {
        std::cout << "  No inventory items currently allocated to this event.\n";
    }
}


// --- Main Function ---
int main() {
    // Set locale for C++ streams to handle potential character issues, though less critical for basic console.
    // This can help with consistent output formatting if special characters were used.
    try {
        std::locale::global(std::locale("")); // Use system's default locale
        std::cout.imbue(std::locale()); 
        std::cin.imbue(std::locale());
    } catch (const std::runtime_error& e) {
        std::cerr << "Warning: Could not set locale. Defaulting to 'C' locale. " << e.what() << std::endl;
    }


    System eventManagementSystem; // Create the main system object
    eventManagementSystem.run();  // Start the application loop

    return 0; // Successful execution
}

