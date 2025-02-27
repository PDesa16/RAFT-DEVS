#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <vector>


using namespace cadmium;

// State struct
template <typename MessageType>
struct BufferState {
    std::queue<MessageType> buffer;
    bool busy = false;

    // Overload operator<< for BufferState, allows us to log state in a cleaner way
    friend std::ostream& operator<<(std::ostream& os, const BufferState<MessageType>& state) {
        os << "BufferState: {busy: " << (state.busy ? "true" : "false") << ", messages: [";
        
        // Iterate through the message queue
        std::queue<std::string> copy = state.buffer;  // Make a copy to avoid modifying the original
        while (!copy.empty()) {
            os << copy.front();
            copy.pop();
            if (!copy.empty()) {
                os << ", ";
            }
        }

        os << "] }";
        return os;
    }

};

// Add Overload operator for >>, for input stream

template <typename MessageType>
// Buffer Atomic Model
class Buffer : public Atomic<BufferState<MessageType>> {
public:
    Port<MessageType> in_port;
    Port<MessageType> out_port;

    Buffer(const std::string& id) : Atomic<BufferState<MessageType>>(id, {}) {}

    void internalTransition(BufferState<MessageType>& s) const override {
        if (!s.buffer.empty()) {
            s.buffer.pop();
        }
        s.busy = !s.buffer.empty();
    }

    void externalTransition(BufferState<MessageType>& s, double e) const override {
        while (!in_port -> empty()) {
            std::vector<MessageType> msgs = in_port -> getBag();
            for (auto msg : msgs) {
                s.buffer.push(msg);
            }
        }
        s.busy = true;
    }

    void output(const BufferState<MessageType>& s) const override {
        if (!s.buffer.empty()) {
            out_port -> addMessage(s.buffer.front());
        }
    }

    double timeAdvance(const BufferState<MessageType>& s) const override {
        return s.busy ? getProcessingDelay() : std::numeric_limits<double>::infinity();
    }

    double getProcessingDelay() const {
        return 0.05;
    }

};

#endif
