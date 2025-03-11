#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <vector>


using namespace cadmium;



enum class DatabaseStatus { IDLE, PROCESSING, READY_TO_OUTPUT };

template <typename DatabaseEntry, typename Query>
struct DatabaseState {
    std::unordered_map<std::string, std::string> database; // Stores key-value pairs
    std::optional<DatabaseEntry> last_query_result;    // Stores last query output
    DatabaseStatus status = DatabaseStatus::IDLE;      // Tracks system state

    // Overload operator<< for BufferState, allows us to log state in a cleaner way
    friend std::ostream& operator<<(std::ostream& os, const DatabaseState<DatabaseEntry, Query>& state) {
        std::string systemStatus;
        switch (state.status)
        {
        case DatabaseStatus::IDLE:
            systemStatus = "IDLE";
            break;
        case DatabaseStatus::PROCESSING:
            systemStatus = "PROCESSING";
            break;

        case DatabaseStatus::READY_TO_OUTPUT:
            systemStatus = "READY TO OUTPUT";
            break;
        
        default:
            break;
        }
        os << "DatabaseStatus: " <<  systemStatus;
        return os;
    }

};



template <typename DatabaseEntry, typename Query>
// Buffer Atomic Model
class Database : public Atomic<DatabaseState<DatabaseEntry, Query>> {
public:

    // // Virtual functions
    // virtual void insertEntry(const DatabaseEntry& databaseEntry) {};
    // virtual std::vector<DatabaseEntry> query(const Query& databaseEntry) {};

    // For Storing 
    Port<DatabaseEntry> in_entry ;
    // For Query 
    Port<Query> in_query;
    // For Output
    Port<std::vector<DatabaseEntry>> out_data;

    Database(const std::string& id) : Atomic<DatabaseState<DatabaseEntry,Query>>(id, {}) {}

    void internalTransition(DatabaseState<DatabaseEntry,Query>& s) const override {
    }

    void externalTransition(DatabaseState<DatabaseEntry,Query>& s, double e) const override {

    }

    void output(const DatabaseState<DatabaseEntry,Query>& s) const override {

    }

    double timeAdvance(const DatabaseState<DatabaseEntry,Query>& s) const override {
        return s.status == DatabaseStatus::PROCESSING ? getProcessingDelay() : std::numeric_limits<double>::infinity();
    }

    double getProcessingDelay() const {
        return 0.05;
    }

};

#endif
