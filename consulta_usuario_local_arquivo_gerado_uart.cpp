#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>

struct Event {
    std::string controllerId;
    std::string payload;
    std::tm timestamp;
};

void readEventsFromFile(const std::string& filename, std::vector<Event>& events) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        Event event;
        std::istringstream iss(line);

        // Ler ID do Controlador
        iss >> event.controllerId;

        // Ler Payload
        std::getline(iss >> std::ws, event.payload, ',');

        // Ler Data/Hora
        iss >> std::get_time(&event.timestamp, "%H:%M:%S");
        if (iss.fail()) {
            std::cerr << "Formato de hora inválido." << std::endl;
            continue;
        }

        // Adicionando mensagem de debug
        std::cout << "Lido: ID=" << event.controllerId << ", Payload=" << event.payload << ", Timestamp=" << std::put_time(&event.timestamp, "%H:%M:%S") << std::endl;

        events.push_back(event);
    }

    inputFile.close();
}

bool isBetween(const Event& event, const std::tm& start, const std::tm& end) {
    return std::difftime(std::mktime(&event.timestamp), std::mktime(&start)) >= 0 &&
           std::difftime(std::mktime(&event.timestamp), std::mktime(&end)) <= 0;
}

void listAllEvents(const std::vector<Event>& events) {
    std::cout << "Todos os eventos:" << std::endl;

    for (const auto& event : events) {
        std::cout << "ID do Controlador: " << event.controllerId
                  << ", Payload: " << event.payload
                  << ", Data/Hora: " << std::put_time(&event.timestamp, "%H:%M:%S")
                  << std::endl;
    }
}

void listEventsInInterval(const std::vector<Event>& events, const std::tm& start, const std::tm& end) {
    std::cout << "Eventos no intervalo de " << std::put_time(&start, "%H:%M:%S") << " a " << std::put_time(&end, "%H:%M:%S") << ":" << std::endl;

    for (const auto& event : events) {
        // Verifica se o evento está no intervalo [start, end]
        if (isBetween(event, start, end)) {
            std::cout << "ID do Controlador: " << event.controllerId
                      << ", Payload: " << event.payload
                      << ", Data/Hora: " << std::put_time(&event.timestamp, "%H:%M:%S")
                      << std::endl;
        }
    }
}

int main() {
    std::vector<Event> events;
    readEventsFromFile("eventList.txt", events);

    if (events.empty()) {
        std::cerr << "Nenhum evento encontrado no arquivo." << std::endl;
        return 1;
    }

    // Opção para listar todos os eventos antes de pedir o intervalo ao usuário
    listAllEvents(events);

    std::cout << "\nEscolha uma opção:" << std::endl;
    std::cout << "1. Listar eventos em um intervalo de datas" << std::endl;

    int option;
    std::cin >> option;

    if (option == 1) {
        std::tm start = {}, end = {};

        std::cout << "Digite a data/hora de início (formato H:MM:SS): ";
        std::cin >> std::get_time(&start, "%H:%M:%S");

        std::cout << "Digite a data/hora de término (formato H:MM:SS): ";
        std::cin >> std::get_time(&end, "%H:%M:%S");

        listEventsInInterval(events, start, end);
    }

    return 0;
}
