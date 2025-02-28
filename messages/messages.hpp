#ifndef MESSAGES_HPP
#define MESSAGES_HPP

template <typename ContentTask>
class BaseMessageContentInterface {
    virtual BaseMessageContentInterface<ContentTask>& getContent() = 0;
    virtual ContentTask getType() = 0;
}


#endif 