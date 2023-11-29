#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <set>
#include "UARTConfig.h"
#include "Event.h"
#include "EventProcessor.h"
#include <atomic>
#include <fstream>
#include <algorithm>
#include <unistd.h>

std::mutex mtx;
std::condition_variable cv;
bool menuShouldBeDisplayed = false;
bool eventReceived = false;
std::string userOption;
std::mutex userInputMutex;
bool userInputComplete = false;
std::string userInput;
std::atomic<bool> exitProgram(false);

std::mutex menuMtx; // Mutex para operações relacionadas ao menu
std::condition_variable menuCv;

void showMenu(const std::vector<Event>& eventList);
void getUserOption();

bool checkIfThreeElementsExist(const std::string& fileName);

void getUserOption() {
    while (true) {
        std::string input;
        std::cout << "Digite sua opção (1-4): ";
        std::getline(std::cin, input);

        {
            std::lock_guard<std::mutex> lock(userInputMutex);
            userOption = input;
            userInputComplete = true;
        }

        menuCv.notify_one();

        if (input == "4") {
            exitProgram = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void readUART(std::vector<Event>& eventList, int& lineIndex, UARTConfig& uart) {
    char buffer[256];
    char lineBuffer[256];
    std::set<std::string> uniqueEvents;

    while (true) {
        int bytesRead = read(uart.getSerialPort(), buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            for (int i = 0; i < bytesRead; ++i) {
                if (buffer[i] == '\n') {
                    lineBuffer[lineIndex] = '\0';
                    std::cout << "Recebido: " << lineBuffer << std::endl;

                    std::string lineString(lineBuffer);

                    std::string payloadData;
                    EventProcessor::processPayload(lineString, payloadData);

                    std::string controllerIdData;
                    EventProcessor::processControllerId(lineString, controllerIdData);

                    std::string timestampData;
                    EventProcessor::processTimestamp(lineString, timestampData);

                    // Verifica se todos os dados do evento são válidos antes de adicioná-lo à lista
                    if (!controllerIdData.empty() && !payloadData.empty() && !timestampData.empty()) {
                        {
                            std::lock_guard<std::mutex> lock(mtx);

                            // Verifica se o evento já existe na lista
                            if (!EventProcessor::isDuplicateEvent(lineString, uniqueEvents)) {
                                Event event;
                                event.controllerId = controllerIdData;
                                event.payload = payloadData;
                                event.timestamp = timestampData;

                                eventList.push_back(event);
                                eventReceived = true;
                                cv.notify_one();
                                std::cout << "Evento adicionado à lista." << std::endl;

                                if (eventList.size() >= 3 && !menuShouldBeDisplayed) {
                                    // Notificar a thread do menu apenas se ainda não foi notificado
                                    menuShouldBeDisplayed = true;
                                    std::cout << "XXXXXXXXX 3 ELEMENTOS NO ARQUIVO TXT: "  << std::endl;
                                    menuCv.notify_one();
                                }

                                uniqueEvents.insert(lineString);  // Adiciona o evento ao conjunto de eventos únicos
                            }
                        }
                    }

                    lineIndex = 0;
                } else {
                    lineBuffer[lineIndex] = buffer[i];
                    ++lineIndex;

                    if (lineIndex >= sizeof(lineBuffer)) {
                        std::cerr << "Aviso: Tamanho do buffer da linha excedido." << std::endl;
                        lineIndex = 0;
                    }
                }
            }
        } else if (bytesRead < 0) {
            std::cerr << "Erro ao ler da porta serial." << std::endl;
            break;
        }
    }
}

void showMenu(const std::vector<Event>& eventList) {
    while (true) {
        std::unique_lock<std::mutex> lock(menuMtx);

        menuCv.wait(lock, [&eventList] { return !userOption.empty() || exitProgram || eventList.size() < 3; });

        if (exitProgram) {
            break;
        }

        if (eventList.size() >= 3 && menuShouldBeDisplayed) {
            // Exibir o menu apenas se a lista de eventos for maior ou igual a 3
            std::cout << "===== Menu =====\n";
            std::cout << "1. Listar eventos em um intervalo de tempo\n";
            std::cout << "2. Calcular tempo total ativo em um intervalo de tempo\n";
            std::cout << "3. Salvar lista de eventos em arquivo\n";
            std::cout << "4. Sair\n";
            std::cout << "================\n";

            cv.notify_one();  // Notificar a thread do menu
            lock.unlock();    // Liberar o mutex antes de aguardar a entrada do usuário
            menuCv.wait(lock, [] { return !userOption.empty() || exitProgram; });
        }

        lock.unlock();  // Liberar o mutex

        // Restante do código do menu...
    }
}

bool checkIfThreeElementsExist(const std::string& fileName) {
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << fileName << std::endl;
        return false;
    }

    std::string line;
    int count = 0;
    while (std::getline(inputFile, line)) {
        ++count;
        if (count >= 3) {
            return true;
        }
    }

    return false;
}

void saveToFileThread(std::vector<Event>& eventList) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [&eventList] { return eventReceived || !eventList.empty(); });

        if (eventReceived || !eventList.empty()) {
            std::vector<Event> eventsToSave = std::move(eventList);
            eventReceived = false;

            lock.unlock();  // Liberar o mutex antes de escrever no arquivo

            if (!eventsToSave.empty()) {
                std::ofstream outputFile("eventList15.txt", std::ios::app);  // Modo de anexo
                if (outputFile.is_open()) {
                    for (const auto& event : eventsToSave) {
                        outputFile << "ID do Controlador: " << event.controllerId
                                   << ", Payload: " << event.payload
                                   << ", Data/Hora: " << event.timestamp << std::endl;
                    }
                    outputFile.close();
                    std::cout << "Lista de eventos acumulada em eventList15.txt\n";
                } else {
                    std::cerr << "Erro ao abrir o arquivo de saída 'eventList15.txt' para escrita." << std::endl;
                }
            }
        }
    }
}

int main() {
    UARTConfig uart("/dev/ttyACM1", B115200);

    if (!uart.openPort() || !uart.configurePort()) {
        return 1;
    }

    std::vector<Event> eventList;

    char buffer[256];
    char lineBuffer[256];
    int lineIndex = 0;

    std::thread uartThread(readUART, std::ref(eventList), std::ref(lineIndex), std::ref(uart));
    std::thread saveToFile(saveToFileThread, std::ref(eventList));
    std::thread userInputThread(getUserOption);

    std::string fileName = "eventList15.txt";
    std::cout << "Checando elementos no arquivo...\n";

    if (checkIfThreeElementsExist(fileName)) {
        std::cout << "Existem pelo menos 3 elementos no arquivo.\n";
        std::unique_lock<std::mutex> lock(menuMtx);
        menuShouldBeDisplayed = true;
        menuCv.notify_one();
    } else {
        std::cout << "Menos de 3 elementos no arquivo.\n";
    }

    while (true) {
        if (exitProgram) {
            break;
        }

        std::unique_lock<std::mutex> lock(menuMtx);

        if (eventList.size() >= 3 && !menuShouldBeDisplayed) {
            menuShouldBeDisplayed = true;
            menuCv.notify_one();
        }

        cv.wait(lock, [] { return !userOption.empty(); });

        if (userOption == "1") {
            std::string startDate, endDate;
            std::cout << "Digite a data de início (AAAA-MM-DD HH:MM:SS): ";
            std::getline(std::cin, startDate);
            std::cout << "Digite a data de término (AAAA-MM-DD HH:MM:SS): ";
            std::getline(std::cin, endDate);
            EventProcessor::listEventsInInterval(eventList, startDate, endDate);
        } else if (userOption == "2") {
            std::string startDate, endDate;
            std::cout << "Digite a data de início (AAAA-MM-DD HH:MM:SS): ";
            std::getline(std::cin, startDate);
            std::cout << "Digite a data de término (AAAA-MM-DD HH:MM:SS): ";
            std::getline(std::cin, endDate);
            EventProcessor::totalActiveTimeInInterval(eventList, startDate, endDate);
        } else if (userOption == "3") {
            EventProcessor::saveEventListToFile(eventList, "eventList15.txt");
        } else if (userOption == "4") {
            exitProgram = true;
        } else {
            std::cout << "Opção inválida. Tente novamente.\n";
        }

        userOption.clear();

        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    uartThread.join();
    saveToFile.join();
    userInputThread.join();

    close(uart.getSerialPort());

    return 0;
}