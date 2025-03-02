#ifndef MESSAGES_HPP
#define MESSAGES_HPP

template <typename ContentTask>
class BaseMessageContentInterface {
    public:
    BaseMessageContentInterface() = default;
    virtual ~BaseMessageContentInterface() = default; 
    virtual BaseMessageContentInterface<ContentTask>& getContent() = 0;
    virtual ContentTask getType() = 0;
    virtual std::string toString() const = 0;
};


#endif 