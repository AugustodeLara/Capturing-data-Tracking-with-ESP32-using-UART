#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>

long long totalActiveTime = 0;
std::string initialRange;
std::string endRange;


int hourInit, minuteInit, secondInit;

int hourEnd, minuteEnd, secondEnd;

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

        std::cerr << ":" << std::endl;
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


std::string removeSpaces(const std::string& str) {
    std::string result;
    for (char ch : str) {
        if (!std::isspace(ch)) {
            result += ch;
        }
    }
    return result;
}

bool isDateTimeInRangeToActiveTime(const std::string& dateTime, const char* startRange, const char* endRange) {
    std::istringstream dateStream(dateTime);
    int hour, minute, second;
    char colon, period;

    dateStream >> hour >> colon >> minute >> colon >> second >> period;

    if (period == 'P' || period == 'p') {
        if (hour != 12) {
            hour += 12;
        }
    } else if (period == 'A' || period == 'a') {
        if (hour == 12) {
            hour = 0;
        }
    }

    struct tm tmDateTime = {};
    tmDateTime.tm_hour = hour;
    tmDateTime.tm_min = minute;
    tmDateTime.tm_sec = second;

    struct tm tmStart = {};
    sscanf(startRange, "%d:%d:%d", &tmStart.tm_hour, &tmStart.tm_min, &tmStart.tm_sec);

    struct tm tmEnd = {};
    sscanf(endRange, "%d:%d:%d", &tmEnd.tm_hour, &tmEnd.tm_min, &tmEnd.tm_sec);

    tmDateTime.tm_year = tmStart.tm_year = tmEnd.tm_year = 0;
    tmDateTime.tm_mon = tmStart.tm_mon = tmEnd.tm_mon = 0;
    tmDateTime.tm_mday = tmStart.tm_mday = tmEnd.tm_mday = 0;

    if (tmDateTime.tm_hour < tmStart.tm_hour ||
        (tmDateTime.tm_hour == tmStart.tm_hour && tmDateTime.tm_min < tmStart.tm_min) ||
        (tmDateTime.tm_hour == tmStart.tm_hour && tmDateTime.tm_min == tmStart.tm_min && tmDateTime.tm_sec < tmStart.tm_sec)) {
        return false;
    }

    if (tmDateTime.tm_hour > tmEnd.tm_hour ||
        (tmDateTime.tm_hour == tmEnd.tm_hour && tmDateTime.tm_min > tmEnd.tm_min) ||
        (tmDateTime.tm_hour == tmEnd.tm_hour && tmDateTime.tm_min == tmEnd.tm_min && tmDateTime.tm_sec > tmEnd.tm_sec)) {
        return false;
    }

    return true;
}

void calculateTotalActiveTime(int targetControllerID, const char* startTimeChars, const char* endTimeChars) {
    const char *filePath = "lista.csv";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    bool insideRange = false;
    long long startTimeInMillis = 0;
    struct std::tm tmDateTime = {};
    std::string dateTimeAux = ""; 

    std::string line;
    while (std::getline(file, line)) {
        size_t controllerPos = line.find("ID do Controlador: ");
        if (controllerPos != std::string::npos) {
            size_t datePos = line.find("Data/Hora: ");
            if (datePos != std::string::npos) {
                std::string controllerIDStr = line.substr(controllerPos + 19, line.find(",", controllerPos) - (controllerPos + 19));
                int currentControllerID = std::stoi(controllerIDStr);

                if (currentControllerID == targetControllerID) {
                    std::string dateTime = line.substr(datePos + 11);
                    if (isDateTimeInRangeToActiveTime(dateTime, startTimeChars, endTimeChars)) {
                    dateTimeAux = dateTime;
                    std::cout << "xxxxxxx -  dateTime " << dateTime << std::endl;
                        if (!insideRange) {
                            insideRange = true;
                            std::cout << "Entrou no intervalo para o controlador " << targetControllerID << std::endl;
                            std::cout << "Entrou no intervalo tempo " << dateTime << std::endl;
                            initialRange = dateTime;
                            std::cout << "xxxxxxx -  initialRange " << initialRange << std::endl;
                            
                           std::istringstream dateStream(initialRange);
			    char colon, period;

			    dateStream >> hourInit >> colon >> minuteInit >> colon >> secondInit >> period;

			    if (period == 'P' || period == 'p') {
				if (hourInit != 12) {
				    hourInit += 12;
				}
			    } else if (period == 'A' || period == 'a') {
				if (hourInit == 12) {
				    hourInit = 0;
				}
			    }

			    struct tm tmDateTime = {};
			    tmDateTime.tm_hour = hourInit;
			    tmDateTime.tm_min = minuteInit;
			    tmDateTime.tm_sec = secondInit;
			    			    
			    std::cout << "XXXXXXX - initialRange " << hourInit << minuteInit << secondInit << std::endl;

                        }
                        // Atualiza o endRange enquanto ainda está dentro do intervalo
                        endRange = dateTimeAux;
                    } else {
                        if (insideRange) {
                            insideRange = false;
                            std::cout << "Saiu do intervalo para o controlador " << targetControllerID << std::endl;
                            std::cout << "Saiu do intervalo  tempo " << dateTimeAux << std::endl;
                            endRange = dateTimeAux;
                             std::istringstream dateStream(endRange);
			    char colon, period;

			    dateStream >> hourEnd >> colon >> minuteEnd >> colon >> secondEnd >> period;

			    if (period == 'P' || period == 'p') {
				if (hourEnd != 12) {
				    hourEnd += 12;
				}
			    } else if (period == 'A' || period == 'a') {
				if (hourEnd == 12) {
				    hourEnd = 0;
				}
			    }

			    struct tm tmDateTime = {};
			    tmDateTime.tm_hour = hourEnd;
			    tmDateTime.tm_min = minuteEnd;
			    tmDateTime.tm_sec = secondEnd;
			    			    
			    std::cout << "XXXXXXX - endRange " << hourEnd << minuteEnd << secondEnd << std::endl;                            
                        }
                        std::cout << "Fora do intervalo para o controlador " << targetControllerID << ": " << dateTime << std::endl;
                    }
                }
            }
        }
    }

    file.close();

    std::cout << "Soma total para o controlador " << targetControllerID << ": "
              << "hours: " <<  (hourEnd - hourInit) << ", "
              << "minutes: " << (minuteEnd - minuteInit) << ", "
              << "seconds: " << (secondEnd - secondInit)  << std::endl;
}

int main() {
    int choice;

    while (true) {
        std::cout << "Escolha uma opção:\n";
        std::cout << "1 - Listar eventos por intervalo\n";
        std::cout << "2 - Tempo ativo do controlador\n";
        std::cout << "0 - Sair\n";
        std::cout << "Digite sua escolha: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                listEventsByInterval();
                break;
            case 2: {
                int targetControllerID;
                std::cout << "Digite o ID do Controlador (ex: 1): ";
                std::cin >> targetControllerID;

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                const int MAX_TIME_LENGTH = 10;  // Tamanho máximo da string de tempo (hh:mm:ss)
                char startTimeChars[MAX_TIME_LENGTH];
                char endTimeChars[MAX_TIME_LENGTH];

                std::cout << "Digite a Data de Início (ex: 0:0:6): ";
                std::cin.getline(startTimeChars, MAX_TIME_LENGTH);

                std::cout << "Digite a Data de Término (ex: 0:0:18): ";
                std::cin.getline(endTimeChars, MAX_TIME_LENGTH);

                totalActiveTime = 0;
                initialRange = "";
                endRange = "";
                calculateTotalActiveTime(targetControllerID, startTimeChars, endTimeChars);
                break;
            }
            case 0:
                std::cout << "Saindo do programa. Até mais!\n";
                return 0;
            default:
                std::cerr << "Opção inválida. Tente novamente.\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
        }
    }

    return 0;
}
