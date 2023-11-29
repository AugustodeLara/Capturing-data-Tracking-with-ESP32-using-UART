#ifndef EVENT_H
#define EVENT_H

#include <string>

class Event {
public:
    std::string timestamp;
    std::string controllerId;
    std::string payload;

    bool operator==(const Event& other) const;
};

#endif // EVENT_H

