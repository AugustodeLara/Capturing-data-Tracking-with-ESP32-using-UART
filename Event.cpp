#include "Event.h"

bool Event::operator==(const Event& other) const {
    return (timestamp == other.timestamp &&
            controllerId == other.controllerId &&
            payload == other.payload);
}

