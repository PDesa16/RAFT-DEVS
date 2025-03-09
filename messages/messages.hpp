#ifndef MESSAGES_HPP
#define MESSAGES_HPP


enum class PacketPayloadType {RAFT}; // Add type here later..

template <typename ContentTask>
class IMessage 
{
public:
    IMessage() = default;
    virtual ~IMessage() = default; 
    virtual ContentTask getType() = 0;
    virtual std::string toString() const = 0;
};


#endif 