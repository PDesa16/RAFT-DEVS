#ifndef DATABASE_MESSAGES_HPP
#define DATABASE_MESSAGES_HPP

#include "../messages.hpp"
#include <string>

enum class DatabaseTask {INSERT, QUERY};

struct DatabaseMessage {
    explicit DatabaseMessage(std::shared_ptr<BaseMessageContentInterface<DatabaseTask>> _content) : content(std::move(_content)) {}
    std::shared_ptr<BaseMessageContentInterface<DatabaseTask>> content;
};

class InsertMetadata {
    public:
        double timeStamp;  // Event timestamp
        std::string eventType;  // Type of event (e.g., "start", "end")
        int sourceID;  // ID of the source component (could be a model, etc.)
    
        // Simple constructor
        InsertMetadata(double ts, const std::string& evtType, int srcID)
            : timeStamp(ts), eventType(evtType), sourceID(srcID) {}
    };

class QueryMetadata {
        public:
            double startTime;  // Start time of the query range
            double endTime;    // End time of the query range
            std::optional<std::string> eventTypeFilter;  // Optional filter for event type (e.g., "start", "end")
            std::optional<int> sourceIDFilter;  // Optional filter for source ID
        
            // Simple constructor
            QueryMetadata(double start, double end) : startTime(start), endTime(end) {}
        
            // Setters for optional filters
            void setEventTypeFilter(const std::string& type) { eventTypeFilter = type; }
            void setSourceIDFilter(int id) { sourceIDFilter = id; }
};

class InsertDatabase : BaseMessageContentInterface<DatabaseTask> {
    InsertMetadata metadata;

    BaseMessageContentInterface<DatabaseTask>& getContent() override { 
        return *this; 
    }

    DatabaseTask getType() override {
        return DatabaseTask::INSERT;
    }
};

class QueryDatabase : BaseMessageContentInterface<DatabaseTask> {
    QueryMetadata metadata;

    BaseMessageContentInterface<DatabaseTask>& getContent() override { 
        return *this; 
    }

    DatabaseTask getType() override {
        return DatabaseTask::QUERY;
    }
};



#endif