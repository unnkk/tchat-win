#include "preprocesses.hpp"

const char* SERVER_IP;
const char* nickname;
char* clientNickname;
bool is_client_connection_closing(std::string msg);

bool is_exit = false;
bool winMagicExit = false;

void receiveMSG(const std::string& current_input, int &socket){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    while(!is_exit){
        recv(socket, buffer, BUFFER_SIZE, 0);
        if(is_client_connection_closing(buffer)){
            is_exit = true;
        }
        if(!winMagicExit){
            std::cout << "\r" << std::string(current_input.size() + 2, ' ') << "\r" << std::flush;
            std::cout << clientNickname << ": " << buffer << std::endl;

            !is_exit ? std::cout << ">" << current_input << std::flush : std::cout << "User disconnected.\nPress any key to end. . ." << std::endl;
        }
    }
}

void userInput(std::string& current_input, int& socket){
    char ch;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    while(!is_exit){
        std::cout << ">";
        current_input.clear();

        while(!is_exit){
            ch = _getch();
            if(!is_exit){
                if (ch == '\r') { // Нажатие Enter
                    std::cout << std::string(current_input.size() + 2, ' ') << "\r";
                    break;
                } else if (ch == '\b') { // Обработка Backspace (127 — код Backspace в Linux)
                    if (current_input.size()) {
                        current_input.pop_back();
                        std::cout << "\b \b" << std::flush; // Удаляем последний символ
                    }
                } else {
                    current_input += ch;
                    std::cout << ch << std::flush;
                }
            }
        }

        current_input.copy(buffer, BUFFER_SIZE);

        if(is_client_connection_closing(buffer)){
            is_exit = true;
            winMagicExit = true;
        }
        send(socket, buffer, BUFFER_SIZE, 0);
        if(!is_exit){
            std::cout << nickname << "(You): " << buffer << std::flush << std::endl;
            memset(buffer, 0, BUFFER_SIZE);
        }
    }
}

int main(int argc, const char* argv[]){
    WSAData wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        std::cerr << "WSAStartup failed!" << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    std::ifstream iFile;
    std::string ip;
    std::string nickstr;

    iFile.open("config.txt");
    if(!iFile.is_open()){
        std::cout << "Cannot find \'config.txt\', using default ip(127.0.0.1) & nickname(User). . ." << std::endl;
        SERVER_IP = DEFAULT_IP;
        nickname = DEFAULT_NICKNAME;
    }else{      
        std::getline(iFile, ip);
        SERVER_IP = ip.c_str();
        std::getline(iFile, nickstr);
        nickname = nickstr.c_str();
    } 

    std::cout << SERVER_IP << std::endl;

    int server, client;
    std::string current_input;
    
    struct sockaddr_in address;
    int addrlen{sizeof(address)};
    char buffer[BUFFER_SIZE];

    client = socket(AF_INET, SOCK_STREAM, 0);
    if(client == INVALID_SOCKET){
        std::cerr << "Failed to create socket!" << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(DEFAULT_PORT);
    address.sin_addr.s_addr = inet_addr(SERVER_IP);
    std::cout << SERVER_IP << std::endl;

    if(connect(client, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR){
        std::cout << SERVER_IP << std::endl;

        std::cout << "Cannot find server " << SERVER_IP << ":" << DEFAULT_PORT <<", starting own. . ." << std::endl;
        server = socket(AF_INET, SOCK_STREAM, 0);
        
        if(server == SOCKET_ERROR){
            std::cerr << "Failed to create server socket!" << std::endl;
            exit(EXIT_FAILURE);
        }

        address.sin_addr.s_addr = INADDR_ANY;

        int ret = bind(server, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));
        if(ret == SOCKET_ERROR){
            std::cout << "Socket binding failed." << std::endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }


        std::cout << "Listetning. . ." << std::endl;
        if(listen(server, 1) == SOCKET_ERROR){
            std::cerr << "Failed to listen." << std::endl;
            exit(EXIT_FAILURE);
        }

        if((client = accept(server, (sockaddr *)&address, (socklen_t *)&addrlen)) == SOCKET_ERROR){
            std::cerr << "Failed to accept." << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "Client connected.\n" << std::endl;
        strcpy(buffer, nickname);
        send(client, buffer, BUFFER_SIZE, 0);
        recv(client, buffer, BUFFER_SIZE, 0);
        clientNickname = buffer;
    }else{
        std::cout << "Connected to " << SERVER_IP << ":" << DEFAULT_PORT << std::endl << std::endl;
        recv(client, buffer, BUFFER_SIZE, 0);
        clientNickname = buffer;
        strcpy(buffer, nickname);
        send(client, buffer, BUFFER_SIZE, 0);
    }

    std::thread receiveTh(receiveMSG, std::ref(current_input), std::ref(client));
    userInput(current_input, client);

    receiveTh.detach();
    receiveTh.~thread();

    closesocket(server);
    closesocket(client);

    std::cout << "\nGoodbye. . .";

    exit(1);
}

bool is_client_connection_closing(std::string msg){
    for(int i = 0; i < msg.size(); i++){
        if(msg[i] == CLIENT_CLOSE_CONNECTION_SYMBOL){
            return true;
        }
    }
    return false;
}