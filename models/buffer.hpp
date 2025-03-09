#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <vector>
#include "../utils/stochastic/random.hpp"


using namespace cadmium;

// State struct
template <typename MessageType>
struct BufferState {
    std::queue<std::shared_ptr<MessageType>> buffer;
    bool busy = false;

    // Overload operator<< for BufferState, allows us to log state in a cleaner way
    friend std::ostream& operator<<(std::ostream& os, const BufferState<MessageType>& state) {
        os << "BufferState: {busy: " << (state.busy ? "true" : "false") << ", messages: [";
        os << "] }";
        return os;
    }

};

// Add Overload operator for >>, for input stream

template <typename MessageType>
// Buffer Atomic Model
class Buffer : public Atomic<BufferState<MessageType>> {
public:
    Port<std::shared_ptr<MessageType>> input_port;
    Port<std::shared_ptr<MessageType>> output_port;


    Buffer(const std::string& id) : Atomic<BufferState<MessageType>>(id, {}) {
        input_port = cadmium::Component::addInPort<std::shared_ptr<MessageType>>("input_buffer");
        output_port = cadmium::Component::addOutPort<std::shared_ptr<MessageType>>("output_buffer");
    }
    

    void internalTransition(BufferState<MessageType>& s) const override {
        if (!s.buffer.empty()) {
            s.buffer.pop();
        }
        s.busy = !s.buffer.empty();
    }

    void externalTransition(BufferState<MessageType>& s, double e) const override {
        while (!input_port -> empty()) {
            std::vector<std::shared_ptr<MessageType>> msgs = input_port -> getBag();
            for (auto msg : msgs) {
                s.buffer.push(msg);
            }
        }
        s.busy = true;
    }

    void output(const BufferState<MessageType>& s) const override {
        if (!s.buffer.empty()) {
            output_port -> addMessage(s.buffer.front());
        }
    }

    double timeAdvance(const BufferState<MessageType>& s) const override {
        return s.busy ? getProcessingDelay() : std::numeric_limits<double>::infinity();
    }

    double getProcessingDelay() const {
        // Keeping it static for now for simplicity 
        return 0.00000001;
    }

};

#endif
