#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>

bool isDateTimeInRange(const std::string& dateTime, const std::tm& startRange, const std::tm& endRange) {
    std::istringstream dateStream(dateTime);
    int hour, minute, second;
    char colon;
    std::string period;

    if (!(dateStream >> hour >> colon >> minute >> colon >> second >> period)) {
        std::cerr << "Erro ao ler hora do formato: " << dateTime << std::endl;
        return false;
    }

    // Converter para o formato de 24 horas se necessário
    if (period == "PM" || period == "pm") {
        if (hour != 12) {
            hour += 12;
        }
    } else if (period == "AM" || period == "am") {
        if (hour == 12) {
            hour = 0;
        }
    }

    std::tm tmDateTime = {};
    tmDateTime.tm_hour = hour;
    tmDateTime.tm_min = minute;
    tmDateTime.tm_sec = second;

    // Definindo informações de data para 0 para garantir que a comparação seja apenas no tempo
    tmDateTime.tm_year = tmDateTime.tm_mon = tmDateTime.tm_mday = 0;

    // Converter std::tm para time_t antes de comparação
    std::time_t timeDateTime = std::mktime(&tmDateTime);
    std::time_t timeStartRange = std::mktime(const_cast<std::tm*>(&startRange));
    std::time_t timeEndRange = std::mktime(const_cast<std::tm*>(&endRange));

    // Adicionando a lógica para comparar com os intervalos de início e fim
    if (timeDateTime < timeStartRange || timeDateTime > timeEndRange) {
        // A data/hora está fora do intervalo
        return false;
    }

    // O evento está dentro do intervalo
    return true;
}

void listEventsByInterval() {
    std::tm startRange = {}, endRange = {};
    std::string input;

    // Leitura da Data de Início
    while (true) {
        std::cout << "Digite a Data de Início (ex: 0:0:6): ";
        std::getline(std::cin, input);
        std::istringstream startStream(input);

        char colon1, colon2;
        if (startStream >> startRange.tm_hour >> colon1 >> startRange.tm_min >> colon2 >> startRange.tm_sec &&
            colon1 == ':' && colon2 == ':') {
            break;
        }

        std::cerr << "Erro ao ler a hora de início. Tente novamente." << std::endl;
        std::cin.clear(); // Limpar o estado de erro
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Limpar a entrada para evitar problemas na próxima leitura
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Leitura da Data de Término
    while (true) {
        std::cout << "Digite a Data de Término (ex: 0:0:18): ";
        std::getline(std::cin, input);
        std::istringstream endStream(input);

        char colon1, colon2;
        if (endStream >> endRange.tm_hour >> colon1 >> endRange.tm_min >> colon2 >> endRange.tm_sec &&
            colon1 == ':' && colon2 == ':') {
            break;
        }

        std::cerr << "Erro ao ler a hora de término. Tente novamente." << std::endl;
        std::cin.clear(); // Limpar o estado de erro
    }

    const char *filePath = "lista4.csv";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t datePos = line.find("Data/Hora: ");
        if (datePos != std::string::npos) {
            std::string dateTime = line.substr(datePos + 11); // Adicionado offset para pegar a data/hora correta

            if (isDateTimeInRange(dateTime, startRange, endRange)) {
                std::cout << line << std::endl;
            }
        }
    }

    file.close();
}

int main() {
    int choice;

    while (true) {
        std::cout << "Escolha uma opção:\n";
        std::cout << "1 - Listar eventos por intervalo\n";
        std::cout << "2 - Outra opção (se desejar)\n";
        std::cout << "0 - Sair\n";
        std::cout << "Digite sua escolha: ";
        std::cin >> choice;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Adicione esta linha

        switch (choice) {
            case 1:
                listEventsByInterval();
                break;
            case 2:
                // Implemente outras opções aqui, se necessário
                break;
            case 0:
                std::cout << "Saindo do programa. Até mais!\n";
                return 0;
            default:
                std::cerr << "Opção inválida. Tente novamente.\n";
                break;
        }
    }

    return 0;
}
