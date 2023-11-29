#ifndef EVENT_PROCESSOR_H
#define EVENT_PROCESSOR_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "Event.h"
#include <set> // Adicione esta linha


class EventProcessor {
public:
    static std::string parseTimestamp(const std::string& timestampString);
    static void processPayload(const std::string& lineString, std::string& payloadData);
    static void processControllerId(const std::string& lineString, std::string& controllerIdData);
    static void processTimestamp(const std::string& lineString, std::string& timestampData);
    static void saveEventListToFile(const std::vector<Event>& eventList, const std::string& filename);
    static bool isDuplicateEvent(const std::string& newEvent, const std::set<std::string>& uniqueEvents);
    static void listEventsInInterval(const std::vector<Event>& eventList, const std::string& startDate, const std::string& endDate);
    static void totalActiveTimeInInterval(const std::vector<Event>& eventList, const std::string& startDate, const std::string& endDate);
};

#endif  // EVENT_PROCESSOR_H

