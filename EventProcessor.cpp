#include "EventProcessor.h"

std::string EventProcessor::parseTimestamp(const std::string& timestampString) {
    return timestampString;
}

void EventProcessor::processPayload(const std::string& lineString, std::string& payloadData) {
    size_t payloadPos = lineString.find("Payload: ");
    payloadData = "";
    if (payloadPos != std::string::npos) {
        payloadPos += 9;
        size_t commaPos = lineString.find(",", payloadPos);
        if (commaPos != std::string::npos) {
            payloadData = lineString.substr(payloadPos, commaPos - payloadPos);
            std::cout << "Payload IF: " << payloadData << std::endl;
        } else {
            payloadData = lineString.substr(payloadPos);
            std::cout << "Payload ELSE: " << payloadData << std::endl;
        }
    }
}

void EventProcessor::processControllerId(const std::string& lineString, std::string& controllerIdData) {
    size_t controllerIdPos = lineString.find("ID do Controlador: ");
    controllerIdData = "";
    if (controllerIdPos != std::string::npos) {
        controllerIdPos += 19;
        size_t commaPosID = lineString.find(",", controllerIdPos);
        if (commaPosID != std::string::npos) {
            controllerIdData = lineString.substr(controllerIdPos, commaPosID - controllerIdPos);
            std::cout << "IDCONTROLLER IF: " << controllerIdData << std::endl;
        } else {
            controllerIdData = lineString.substr(controllerIdPos);
            std::cout << "IDCONTROLLER ELSE: " << controllerIdData << std::endl;
        }
    }
}

void EventProcessor::processTimestamp(const std::string& lineString, std::string& timestampData) {
    size_t timestampPos = lineString.find("Data/Hora: ");
    timestampData = "";
    if (timestampPos != std::string::npos) {
        timestampPos += 11;
        timestampData = lineString.substr(timestampPos);
    }
}

void EventProcessor::saveEventListToFile(const std::vector<Event>& eventList, const std::string& filename) {
    std::ofstream outputFile(filename, std::ios::app);  // Modo de anexo

    if (!outputFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de saída '" << filename << "' para escrita." << std::endl;
        return;
    }

    for (const auto& event : eventList) {
        outputFile << "ID do Controlador: " << event.controllerId
                   << ", Payload: " << event.payload
                   << ", Data/Hora: " << event.timestamp << std::endl;
    }

    outputFile.close();
    std::cout << "Lista de eventos acumulada em " << filename << std::endl;
}

bool EventProcessor::isDuplicateEvent(const std::string& newEvent, const std::set<std::string>& uniqueEvents) {
    return uniqueEvents.count(newEvent) > 0;
}

void EventProcessor::listEventsInInterval(const std::vector<Event>& eventList, const std::string& startDate, const std::string& endDate) {
    // Implementação para listar eventos no intervalo de datas
    std::cout << "Função listEventsInInterval não implementada.\n";
}

void EventProcessor::totalActiveTimeInInterval(const std::vector<Event>& eventList, const std::string& startDate, const std::string& endDate) {
    // Implementação para calcular o tempo total ativo no intervalo de datas
    std::cout << "Função totalActiveTimeInInterval não implementada.\n";
}

