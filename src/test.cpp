#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>      // Для read()
#include <termios.h>     // Для управления режимами терминала

std::mutex console_mutex;
bool keep_running = true;

// Настраиваем терминал для посимвольного ввода
void setNonCanonicalMode(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);  // Получаем текущие настройки терминала
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Отключаем канонический режим и эхо ввода
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Восстанавливаем старые настройки
    }
}

void displayIncomingMessage(const std::string& message, const std::string& current_input) {
    std::lock_guard<std::mutex> lock(console_mutex);

    // Очищаем текущую строку ввода, чтобы отобразить новое сообщение
    std::cout << "\r" << std::string(current_input.size() + 2, ' ') << "\r" << std::flush;

    // Отображаем новое сообщение
    std::cout << message << "\n";

    // Восстанавливаем приглашение для ввода и текущий ввод
    std::cout << "> " << current_input << std::flush;
}

void receiveMessages(std::string& current_input) {
    int counter = 0;
    while (keep_running) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        displayIncomingMessage("Новое сообщение " + std::to_string(counter++), current_input);
    }
}

void userInput(std::string& input) {
    char ch;

    setNonCanonicalMode(true); // Включаем посимвольный ввод

    while (keep_running) {
        std::cout << "> ";
        input.clear();

        while (true) {
            read(STDIN_FILENO, &ch, 1); // Считываем один символ

            if (ch == '\n') { // Нажатие Enter
                std::cout << "\n";
                break;
            } else if (ch == 127 || ch == '\b') { // Обработка Backspace (127 — код Backspace в Linux)
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b" << std::flush; // Удаляем последний символ
                }
            } else {
                input += ch;
                std::cout << ch << std::flush;
            }
        }

        // Отображаем отправленное сообщение
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\rВы: " << input << std::flush << std::endl;
        }
    }

    setNonCanonicalMode(false); // Отключаем посимвольный режим перед завершением
}

int main() {
    std::string current_input; // Хранение текущего ввода пользователя

    // Поток для получения сообщений
    std::thread receiveThread(receiveMessages, std::ref(current_input));

    userInput(current_input);

    keep_running = false;
    receiveThread.join();
    return 0;
}
