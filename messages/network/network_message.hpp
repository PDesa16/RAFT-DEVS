#ifndef NETWORK_MESSAGES_HPP
#define NETWORK_MESSAGES_HPP

#include "../messages.hpp"
#include <string>

class Packet {
    public:
        Packet(std::shared_ptr<IMessage<PacketPayloadType>> _payload) : payload(std::move(_payload)) {}
        Packet(std::shared_ptr<IMessage<PacketPayloadType>> _payload, std::string _destination, std::string _source) :
        payload(std::move(_payload)), destination(_destination), source(_source) {}
        std::shared_ptr<IMessage<PacketPayloadType>> payload;
        std::string destination; 
        std::string source;
        double timestamp;

        friend std::ostream& operator<<(std::ostream& os, const Packet& msg) {
            os << "Packet";
            return os;
        }
};


struct PacketEvent {
    public:
        PacketEvent(std::shared_ptr<Packet> _packet, double _delay, double _dispatchTime) : packet(std::move(_packet)), delay(_delay),
        dispatchTime(_dispatchTime) {}
        std::shared_ptr<Packet> packet;
        double delay;
        double dispatchTime;

        // Min check
        bool operator<(const PacketEvent& b) const {
            return (this->dispatchTime + this->delay) > (b.dispatchTime + b.delay); 
        }

};

struct ComparePacketEvent {
    bool operator()(const std::shared_ptr<PacketEvent>& a, const std::shared_ptr<PacketEvent>& b) const {
        return *a < *b;  // Dereference shared_ptr to compare PacketEvent objects
    }
};



#endif