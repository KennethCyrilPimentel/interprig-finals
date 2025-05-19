#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <limits>

using namespace std;

// Forward declarations
class User;
class Event;

// Constants
const int MAX_USERS = 100;
const int MAX_EVENTS = 100;
const int MAX_STR_LEN = 100;

// Exception classes
class ValidationException : public exception {
private:
    const char* message;
public:
    ValidationException(const char* msg) : message(msg) {}
    const char* what() const noexcept override {
        return message;
    }
};

class AuthException : public exception {
private:
    const char* message;
public:
    AuthException(const char* msg) : message(msg) {}
    const char* what() const noexcept override {
        return message;
    }
};

class DatabaseException : public exception {
private:
    const char* message;
public:
    DatabaseException(const char* msg) : message(msg) {}
    const char* what() const noexcept override {
        return message;
    }
};

// User class
class User {
protected:
    int id;
    char username[MAX_STR_LEN];
    char password[MAX_STR_LEN];
    char role[MAX_STR_LEN];
    bool isLoggedIn;

public:
    User() : id(0), isLoggedIn(false) {
        strcpy(username, "");
        strcpy(password, "");
        strcpy(role, "user");
    }

    User(const char* uname, const char* pwd, const char* userRole) : isLoggedIn(false) {
        setId(rand() % 9000 + 1000); // Generate random ID between 1000-9999
        setUsername(uname);
        setPassword(pwd);
        setRole(userRole);
    }

    // Getters
    int getId() const { return id; }
    const char* getUsername() const { return username; }
    const char* getPassword() const { return password; }
    const char* getRole() const { return role; }
    bool getIsLoggedIn() const { return isLoggedIn; }

    // Setters with validation
    void setId(int newId) {
        if (newId <= 0) {
            throw ValidationException("ID must be positive");
        }
        id = newId;
    }

    void setUsername(const char* uname) {
        if (strlen(uname) < 4 || strlen(uname) >= MAX_STR_LEN) {
            throw ValidationException("Username must be between 4-100 characters");
        }
        strcpy(username, uname);
    }

    void setPassword(const char* pwd) {
        if (strlen(pwd) < 6 || strlen(pwd) >= MAX_STR_LEN) {
            throw ValidationException("Password must be between 6-100 characters");
        }
        strcpy(password, pwd);
    }

    void setRole(const char* userRole) {
        if (strcmp(userRole, "admin") != 0 && strcmp(userRole, "user") != 0) {
            throw ValidationException("Role must be either 'admin' or 'user'");
        }
        strcpy(role, userRole);
    }

    void setIsLoggedIn(bool status) { isLoggedIn = status; }

    virtual void displayMenu() = 0; // Pure virtual function for polymorphism

    // Login method
    bool login(const char* uname, const char* pwd) {
        if (strcmp(username, uname) == 0 && strcmp(password, pwd) == 0) {
            isLoggedIn = true;
            return true;
        }
        return false;
    }

    void logout() {
        setIsLoggedIn(false);
    }
};

// Admin class
class Admin : public User {
public:
    Admin(const char* uname, const char* pwd) : User(uname, pwd, "admin") {}

    void displayMenu() override {
        cout << "\nAdmin Menu:\n";
        cout << "1. Create Event\n";
        cout << "2. View All Events\n";
        cout << "3. Update Event\n";
        cout << "4. Delete Event\n";
        cout << "5. View All Users\n";
        cout << "6. Logout\n";
        cout << "Enter your choice: ";
    }
};

// Regular User class
class RegularUser : public User {
public:
    RegularUser(const char* uname, const char* pwd) : User(uname, pwd, "user") {}

    void displayMenu() override {
        cout << "\nUser Menu:\n";
        cout << "1. View All Events\n";
        cout << "2. Register for Event\n";
        cout << "3. View My Events\n";
        cout << "4. Logout\n";
        cout << "Enter your choice: ";
    }
};

// Event class
class Event {
private:
    int id;
    char name[MAX_STR_LEN];
    char description[MAX_STR_LEN];
    char date[MAX_STR_LEN];
    char time[MAX_STR_LEN];
    int capacity;
    int registeredUsers[MAX_USERS];
    int registeredCount;

public:
    Event() : id(0), capacity(0), registeredCount(0) {
        strcpy(name, "");
        strcpy(description, "");
        strcpy(date, "");
        strcpy(time, "");
        for (int i = 0; i < MAX_USERS; i++) {
            registeredUsers[i] = 0;
        }
    }

    Event(const char* evtName, const char* desc, const char* evtDate, const char* evtTime, int cap) 
        : registeredCount(0) {
        setId(rand() % 9000 + 1000); // Generate random ID between 1000-9999
        setName(evtName);
        setDescription(desc);
        setDate(evtDate);
        setTime(evtTime);
        setCapacity(cap);
        for (int i = 0; i < MAX_USERS; i++) {
            registeredUsers[i] = 0;
        }
    }

    // Getters
    int getId() const { return id; }
    const char* getName() const { return name; }
    const char* getDescription() const { return description; }
    const char* getDate() const { return date; }
    const char* getTime() const { return time; }
    int getCapacity() const { return capacity; }
    int getRegisteredCount() const { return registeredCount; }

    // Setters with validation
    void setId(int newId) {
        if (newId <= 0) {
            throw ValidationException("Event ID must be positive");
        }
        id = newId;
    }

    void setName(const char* evtName) {
        if (strlen(evtName) < 3 || strlen(evtName) >= MAX_STR_LEN) {
            throw ValidationException("Event name must be between 3-100 characters");
        }
        strcpy(name, evtName);
    }

    void setDescription(const char* desc) {
        if (strlen(desc) >= MAX_STR_LEN) {
            throw ValidationException("Description must be less than 100 characters");
        }
        strcpy(description, desc);
    }

    void setDate(const char* evtDate) {
        // Simple date format validation (MM/DD/YYYY)
        if (strlen(evtDate) != 10 || evtDate[2] != '/' || evtDate[5] != '/') {
            throw ValidationException("Date must be in MM/DD/YYYY format");
        }
        strcpy(date, evtDate);
    }

    void setTime(const char* evtTime) {
        // Simple time format validation (HH:MM)
        if (strlen(evtTime) != 5 || evtTime[2] != ':') {
            throw ValidationException("Time must be in HH:MM format");
        }
        strcpy(time, evtTime);
    }

    void setCapacity(int cap) {
        if (cap <= 0) {
            throw ValidationException("Capacity must be positive");
        }
        capacity = cap;
    }

    // Register a user for this event
    bool registerUser(int userId) {
        if (registeredCount >= capacity) {
            return false; // Event is full
        }
        
        // Check if user is already registered
        for (int i = 0; i < registeredCount; i++) {
            if (registeredUsers[i] == userId) {
                return false;
            }
        }
        
        registeredUsers[registeredCount++] = userId;
        return true;
    }

    // Check if a user is registered for this event
    bool isUserRegistered(int userId) const {
        for (int i = 0; i < registeredCount; i++) {
            if (registeredUsers[i] == userId) {
                return true;
            }
        }
        return false;
    }

    // Display event details
    void display() const {
        cout << "\nEvent ID: " << id << "\n";
        cout << "Name: " << name << "\n";
        cout << "Description: " << description << "\n";
        cout << "Date: " << date << "\n";
        cout << "Time: " << time << "\n";
        cout << "Capacity: " << capacity << "\n";
        cout << "Registered: " << registeredCount << "\n";
    }
};

// Database class (Singleton)
class Database {
private:
    static Database* instance;
    User* users[MAX_USERS];
    Event* events[MAX_EVENTS];
    int userCount;
    int eventCount;

    // Private constructor for singleton
    Database() : userCount(0), eventCount(0) {
        // Initialize with some default data
        addUser(new Admin("admin", "admin123"));
        addUser(new RegularUser("user1", "user123"));
        addUser(new RegularUser("user2", "user123"));
        
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char date[11];
        sprintf(date, "%02d/%02d/%04d", 1 + ltm->tm_mon, ltm->tm_mday, 1900 + ltm->tm_year);
        
        addEvent(new Event("Tech Conference", "Annual technology conference", date, "09:00", 100));
        addEvent(new Event("Music Festival", "Summer music festival", date, "18:00", 500));
    }

public:
    // Get singleton instance
    static Database* getInstance() {
        if (!instance) {
            instance = new Database();
        }
        return instance;
    }

    // Add a user to the database
    void addUser(User* user) {
        if (userCount >= MAX_USERS) {
            throw DatabaseException("Maximum user capacity reached");
        }
        users[userCount++] = user;
    }

    // Add an event to the database
    void addEvent(Event* event) {
        if (eventCount >= MAX_EVENTS) {
            throw DatabaseException("Maximum event capacity reached");
        }
        events[eventCount++] = event;
    }

    // Find user by username
    User* findUserByUsername(const char* username) {
        for (int i = 0; i < userCount; i++) {
            if (strcmp(users[i]->getUsername(), username) == 0) {
                return users[i];
            }
        }
        return nullptr;
    }

    // Find event by ID
    Event* findEventById(int id) {
        for (int i = 0; i < eventCount; i++) {
            if (events[i]->getId() == id) {
                return events[i];
            }
        }
        return nullptr;
    }

    // Get all users
    User** getAllUsers() { return users; }
    int getUserCount() const { return userCount; }

    // Get all events
    Event** getAllEvents() { return events; }
    int getEventCount() const { return eventCount; }

    // Delete an event
    bool deleteEvent(int id) {
        for (int i = 0; i < eventCount; i++) {
            if (events[i]->getId() == id) {
                delete events[i];
                // Shift remaining elements
                for (int j = i; j < eventCount - 1; j++) {
                    events[j] = events[j + 1];
                }
                eventCount--;
                return true;
            }
        }
        return false;
    }
};

// Initialize static member
Database* Database::instance = nullptr;

// Authentication strategy interface
class AuthStrategy {
public:
    virtual User* authenticate() = 0;
    virtual ~AuthStrategy() {}
};

// Login strategy
class LoginStrategy : public AuthStrategy {
    public:
        User* authenticate() override {
            char username[MAX_STR_LEN];
            char password[MAX_STR_LEN];
            
            cout << "\nLogin\n";  // Correct label
            cout << "Username: ";
            cin.getline(username, MAX_STR_LEN);
            
            cout << "Password: ";
            cin.getline(password, MAX_STR_LEN);
            
            Database* db = Database::getInstance();
            User* user = db->findUserByUsername(username);
            
            if (user && user->login(username, password)) {
                return user;
            }
            
            throw AuthException("Invalid username or password");
        }
    };

// Registration strategy
class RegisterStrategy : public AuthStrategy {
    public:
        User* authenticate() override {
            char username[MAX_STR_LEN];
            char password[MAX_STR_LEN];
            char confirmPassword[MAX_STR_LEN];
            char role[MAX_STR_LEN];
            
            cout << "\nRegister\n";
            
            // Get username
            while (true) {
                try {
                    cout << "Username (4-100 chars): ";
                    cin.getline(username, MAX_STR_LEN);
                    
                    Database* db = Database::getInstance();
                    if (db->findUserByUsername(username)) {
                        throw ValidationException("Username already exists");
                    }
                    
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
            
            // Get password
            while (true) {
                try {
                    cout << "Password (6-100 chars): ";
                    cin.getline(password, MAX_STR_LEN);
                    
                    cout << "Confirm Password: ";
                    cin.getline(confirmPassword, MAX_STR_LEN);
                    
                    if (strcmp(password, confirmPassword) != 0) {
                        throw ValidationException("Passwords do not match");
                    }
                    
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
            
            // Get role
            while (true) {
                try {
                    cout << "Role (admin/user): ";
                    cin.getline(role, MAX_STR_LEN);
                    
                    if (strcmp(role, "admin") != 0 && strcmp(role, "user") != 0) {
                        throw ValidationException("Role must be either 'admin' or 'user'");
                    }
                    
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
            
            // Create user
            User* newUser;
            if (strcmp(role, "admin") == 0) {
                newUser = new Admin(username, password);
            } else {
                newUser = new RegularUser(username, password);
            }
            
            Database* db = Database::getInstance();
            db->addUser(newUser);
            
            // Log in the user and return immediately
            newUser->login(username, password);
            cout << "Registration and login successful!\n";
            return newUser;
        }
    };

// Authentication context
class AuthContext {
private:
    AuthStrategy* strategy;
    
public:
    AuthContext(AuthStrategy* strat) : strategy(strat) {}
    ~AuthContext() { delete strategy; }
    
    User* executeStrategy() {
        return strategy->authenticate();
    }
};

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Helper functions
int getNumericInput(int min, int max) {
    int input;
    while (true) {
        cin >> input;
        if (cin.fail() || input < min || input > max) {
            clearInputBuffer();
            cout << "Invalid input. Please enter a number between " << min << " and " << max << ": ";
        } else {
            clearInputBuffer();
            return input;
        }
    }
}

bool getYesNoInput() {
    char input;
    while (true) {
        cout << " (y/n): ";
        cin >> input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (input == 'y' || input == 'Y') {
            return true;
        } else if (input == 'n' || input == 'N') {
            return false;
        } else {
            cout << "Invalid input. Please enter 'y' or 'n': ";
        }
    }
}

// Main application
class EventManagementSystem {
public:
void run() {
    cout << "Event Management System\n";
    
    User* currentUser = nullptr;
    
    while (true) {
        if (!currentUser) {
            currentUser = showAuthMenu();
            if (!currentUser) continue;
        }
        
        try {
            if (strcmp(currentUser->getRole(), "admin") == 0) {
                adminMenu(currentUser);
            } else {
                userMenu(currentUser);
            }
            
            // After logout, reset currentUser to show auth menu again
            if (!currentUser->getIsLoggedIn()) {
                currentUser = nullptr;
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
            // On error, reset to auth menu
            currentUser = nullptr;
        }
    }
}
    
private:
User* showAuthMenu() {
    while (true) {  // Keep showing menu until valid choice or exit
        cout << "\nEvent Management System\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        
        int choice = getNumericInput(1, 3);
        
        try {
            switch (choice) {
                case 1: {
                    AuthContext context(new LoginStrategy());
                    User* user = context.executeStrategy();
                    if (user) {
                        cout << "\nLogin successful!\n";
                        return user;  // Return logged in user
                    }
                    break;
                }
                case 2: {
                    AuthContext context(new RegisterStrategy());
                    User* user = context.executeStrategy();
                    if (user) {
                        cout << "\nRegistration and login successful!\n";
                        return user;  // Return registered and logged in user
                    }
                    break;
                }
                case 3:
                    cout << "Goodbye!\n";
                    exit(0);
                default:
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } catch (const AuthException& e) {
            cout << "Authentication failed: " << e.what() << "\n";
        } catch (const ValidationException& e) {
            cout << "Validation error: " << e.what() << "\n";
        }
    }
}
    
    void adminMenu(User* user) {
        Database* db = Database::getInstance();
        
        while (user->getIsLoggedIn()) {
            user->displayMenu();
            int choice = getNumericInput(1, 6);
            
            switch (choice) {
                case 1: createEvent(); break;
                case 2: viewAllEvents(); break;
                case 3: updateEvent(); break;
                case 4: deleteEvent(); break;
                case 5: viewAllUsers(db); break;
                case 6: 
                    user->logout(); 
                    cout << "Logged out successfully.\n";
                    break;
                default:
                    cout << "Invalid choice.\n";
            }
        }
    }
    
    void userMenu(User* user) {
        Database* db = Database::getInstance();
        
        while (user->getIsLoggedIn()) {
            user->displayMenu();
            int choice = getNumericInput(1, 4);
            
            switch (choice) {
                case 1: viewAllEvents(); break;
                case 2: registerForEvent(user); break;
                case 3: viewUserEvents(user); break;
                case 4: 
                    user->logout(); 
                    cout << "Logged out successfully.\n";
                    break;
                default:
                    cout << "Invalid choice.\n";
            }
        }
    }
    
    void createEvent() {
        Database* db = Database::getInstance();
        
        char name[MAX_STR_LEN];
        char description[MAX_STR_LEN];
        char date[MAX_STR_LEN];
        char time[MAX_STR_LEN];
        int capacity;
        
        cout << "\nCreate New Event\n";
        
        // Get event name
        while (true) {
            try {
                cout << "Event Name: ";
                cin.getline(name, MAX_STR_LEN);
                break;
            } catch (const ValidationException& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
        
        // Get description
        while (true) {
            try {
                cout << "Description: ";
                cin.getline(description, MAX_STR_LEN);
                break;
            } catch (const ValidationException& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
        
        // Get date
        while (true) {
            try {
                cout << "Date (MM/DD/YYYY): ";
                cin.getline(date, MAX_STR_LEN);
                break;
            } catch (const ValidationException& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
        
        // Get time
        while (true) {
            try {
                cout << "Time (HH:MM): ";
                cin.getline(time, MAX_STR_LEN);
                break;
            } catch (const ValidationException& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
        
        // Get capacity
        while (true) {
            try {
                cout << "Capacity: ";
                capacity = getNumericInput(1, 10000);
                break;
            } catch (const ValidationException& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
        
        Event* newEvent = new Event(name, description, date, time, capacity);
        db->addEvent(newEvent);
        
        cout << "Event created successfully!\n";
        newEvent->display();
    }
    
    void viewAllEvents() {
        Database* db = Database::getInstance();
        Event** events = db->getAllEvents();
        int count = db->getEventCount();
        
        cout << "\nAll Events (" << count << ")\n";
        
        if (count == 0) {
            cout << "No events found.\n";
            return;
        }
        
        for (int i = 0; i < count; i++) {
            events[i]->display();
        }
    }
    
    void updateEvent() {
        Database* db = Database::getInstance();
        
        cout << "\nUpdate Event\n";
        cout << "Enter Event ID to update: ";
        int id = getNumericInput(1000, 9999);
        
        Event* event = db->findEventById(id);
        if (!event) {
            cout << "Event not found.\n";
            return;
        }
        
        cout << "Current event details:\n";
        event->display();
        
        char name[MAX_STR_LEN];
        char description[MAX_STR_LEN];
        char date[MAX_STR_LEN];
        char time[MAX_STR_LEN];
        int capacity;
        
        // Update name
        cout << "Update name? Current: " << event->getName() << "\n";
        if (getYesNoInput()) {
            while (true) {
                try {
                    cout << "New name: ";
                    cin.getline(name, MAX_STR_LEN);
                    event->setName(name);
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
        }
        
        // Update description
        cout << "Update description? Current: " << event->getDescription() << "\n";
        if (getYesNoInput()) {
            while (true) {
                try {
                    cout << "New description: ";
                    cin.getline(description, MAX_STR_LEN);
                    event->setDescription(description);
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
        }
        
        // Update date
        cout << "Update date? Current: " << event->getDate() << "\n";
        if (getYesNoInput()) {
            while (true) {
                try {
                    cout << "New date (MM/DD/YYYY): ";
                    cin.getline(date, MAX_STR_LEN);
                    event->setDate(date);
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
        }
        
        // Update time
        cout << "Update time? Current: " << event->getTime() << "\n";
        if (getYesNoInput()) {
            while (true) {
                try {
                    cout << "New time (HH:MM): ";
                    cin.getline(time, MAX_STR_LEN);
                    event->setTime(time);
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
        }
        
        // Update capacity
        cout << "Update capacity? Current: " << event->getCapacity() << "\n";
        if (getYesNoInput()) {
            while (true) {
                try {
                    cout << "New capacity: ";
                    capacity = getNumericInput(1, 10000);
                    event->setCapacity(capacity);
                    break;
                } catch (const ValidationException& e) {
                    cout << "Error: " << e.what() << "\n";
                }
            }
        }
        
        cout << "Event updated successfully!\n";
        event->display();
    }
    
    void deleteEvent() {
        Database* db = Database::getInstance();
        
        cout << "\nDelete Event\n";
        cout << "Enter Event ID to delete: ";
        int id = getNumericInput(1000, 9999);
        
        Event* event = db->findEventById(id);
        if (!event) {
            cout << "Event not found.\n";
            return;
        }
        
        cout << "You are about to delete this event:\n";
        event->display();
        cout << "Are you sure you want to delete this event?\n";
        
        if (getYesNoInput()) {
            if (db->deleteEvent(id)) {
                cout << "Event deleted successfully.\n";
            } else {
                cout << "Failed to delete event.\n";
            }
        } else {
            cout << "Deletion cancelled.\n";
        }
    }
    
    void viewAllUsers(Database* db) {
        User** users = db->getAllUsers();
        int count = db->getUserCount();
        
        cout << "\nAll Users (" << count << ")\n";
        
        if (count == 0) {
            cout << "No users found.\n";
            return;
        }
        
        for (int i = 0; i < count; i++) {
            cout << "\nUser ID: " << users[i]->getId() << "\n";
            cout << "Username: " << users[i]->getUsername() << "\n";
            cout << "Role: " << users[i]->getRole() << "\n";
        }
    }
    
    void registerForEvent(User* user) {
        Database* db = Database::getInstance();
        
        cout << "\nRegister for Event\n";
        cout << "Enter Event ID: ";
        int id = getNumericInput(1000, 9999);
        
        Event* event = db->findEventById(id);
        if (!event) {
            cout << "Event not found.\n";
            return;
        }
        
        if (event->isUserRegistered(user->getId())) {
            cout << "You are already registered for this event.\n";
            return;
        }
        
        if (event->registerUser(user->getId())) {
            cout << "Successfully registered for the event!\n";
        } else {
            cout << "Event is full. Registration failed.\n";
        }
    }
    
    void viewUserEvents(User* user) {
        Database* db = Database::getInstance();
        Event** events = db->getAllEvents();
        int count = db->getEventCount();
        int userId = user->getId();
        bool found = false;
        
        cout << "\nYour Registered Events\n";
        
        for (int i = 0; i < count; i++) {
            if (events[i]->isUserRegistered(userId)) {
                events[i]->display();
                found = true;
            }
        }
        
        if (!found) {
            cout << "You are not registered for any events.\n";
        }
    }
};

int main() {
    srand(time(0)); // Seed for random ID generation
    
    EventManagementSystem app;
    app.run();
    
    return 0;
}