#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <limits>
#include <cctype>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

// ANSI 색상 코드
#define RESET   "\033[0m"
#define GREEN   "\033[38;2;0;220;0m"      // 목 - 초록
#define RED     "\033[38;2;255;60;60m"    // 화 - 빨강
#define YELLOW  "\033[38;2;255;230;0m"    // 토 - 노랑
#define PURPLE  "\033[38;2;180;100;255m"  // 금 - 보라
#define BLUE    "\033[38;2;50;170;255m"   // 수 - 파랑

enum Wuxing {
    WOOD = 1,   // 목(木)
    FIRE,       // 화(火)
    EARTH,      // 토(土)
    METAL,      // 금(金)
    WATER       // 수(水)
};

static const std::vector<std::string> names = {
    "", "목(木)", "화(火)", "토(土)", "금(金)", "수(水)"
};

static const std::vector<std::string> colors = {
    "", GREEN, RED, YELLOW, PURPLE, BLUE
};

std::string getName(int choice) {
    if (choice >= 1 && choice <= 5) return names[choice];
    return "???";
}

std::string getColor(int choice) {
    if (choice >= 1 && choice <= 5) return colors[choice];
    return RESET;
}

bool enableWindowsVirtualTerminal() {
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return false;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) return false;
    return true;
#else
    return true;
#endif
}

bool stdoutIsTerminal() {
#if defined(_WIN32) || defined(_WIN64)
    return isatty(fileno(stdout));
#else
    return isatty(fileno(stdout));
#endif
}

bool supportsColor() {
    if (!stdoutIsTerminal()) return false;
#if defined(_WIN32) || defined(_WIN64)
    return enableWindowsVirtualTerminal();
#else
    return true;
#endif
}

void clearScreen() {
#if defined(_WIN32) || defined(_WIN64)
    std::system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
}

void printCircle(bool colorEnabled, const std::string& color, const std::string& name) {
    if (colorEnabled) std::cout << color;
    std::cout <<
        "      ███████████      \n"
        "    ████████████████    \n"
        "   ███████████████████   \n"
        "  █████████████████████  \n"
        "  █████████████████████  \n"
        "  █████████████████████  \n"
        "   ███████████████████   \n"
        "    ████████████████    \n"
        "      ███████████      \n";
    std::cout << "         " << name << "         ";
    if (colorEnabled) std::cout << RESET;
    std::cout << "\n\n";
}

// 결과 비교: 반환값: 0 무승부, 1 플레이어 승, -1 플레이어 패
// reason: 한 줄로 왜 승패가 났는지 설명
int compareWithReason(int player, int computer, std::string &reason) {
    if (player == computer) {
        reason = "동일한 기운, 비김.";
        return 0;
    }
    // 목 -> 토, 토 -> 수, 수 -> 화, 화 -> 금, 금 -> 목 (이김)
    if ((player == WOOD && computer == EARTH) ||
        (player == EARTH && computer == WATER) ||
        (player == WATER && computer == FIRE) ||
        (player == FIRE && computer == METAL) ||
        (player == METAL && computer == WOOD)) {
        // 플레이어가 이김
        if (player == WOOD && computer == EARTH) reason = "목(木)이 토(土)를 뚫고 자랍니다.";
        else if (player == EARTH && computer == WATER) reason = "토(土)가 수(水)를 가두어 흡수합니다.";
        else if (player == WATER && computer == FIRE) reason = "수(水)가 화(火)를 끕니다.";
        else if (player == FIRE && computer == METAL) reason = "화(火)가 금(金)을 녹입니다.";
        else if (player == METAL && computer == WOOD) reason = "금(金)이 목(木)을 자릅니다.";
        return 1;
    } else {
        // 플레이어 패
        if (computer == WOOD && player == EARTH) reason = "목(木)이 토(土)를 뚫고 자랍니다.";
        else if (computer == EARTH && player == WATER) reason = "토(土)가 수(水)를 가두어 흡수합니다.";
        else if (computer == WATER && player == FIRE) reason = "수(水)가 화(火)를 끕니다.";
        else if (computer == FIRE && player == METAL) reason = "화(火)가 금(金)을 녹입니다.";
        else if (computer == METAL && player == WOOD) reason = "금(金)이 목(木)을 자릅니다.";
        else reason = "오행의 흐름에 의해 패배했습니다.";
        return -1;
    }
}

// 숫자 앞뒤 공백 제거
static inline std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// 단축키 문자 -> 선택값(1..5). 없으면 0
int shortcutToChoice(char ch) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    switch (ch) {
        case 'G': return WOOD;   // Green -> 목
        case 'R': return FIRE;   // Red -> 화
        case 'Y': return EARTH;  // Yellow -> 토
        case 'P': return METAL;  // Purple -> 금
        case 'B': return WATER;  // Blue -> 수
        default:  return 0;
    }
}

int main() {
    bool colorEnabled = supportsColor();

    std::random_device rd;
    auto seed = rd() ^ static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(1, 5);

    int player = 0, computer = 0;
    int scorePlayer = 0, scoreComputer = 0, scoreTie = 0;
    std::string line;

    std::cout << "════════════════════════════════\n";
    std::cout << "   색깔 원 오행 가위바위보 완성!\n";
    std::cout << "════════════════════════════════\n\n";
    std::cout << "입력 안내: 1:목 2:화 3:토 4:금 5:수\n";
    std::cout << "단축키: G(목), R(화), Y(토), P(금), B(수)\n";
    std::cout << "게임 종료: Q\n\n";

    while (true) {
        clearScreen();
        std::cout << "현재 스코어 -> 당신: " << scorePlayer << "   상대: " << scoreComputer << "   무승부: " << scoreTie << "\n\n";
        std::cout << "1: 목(木)   2: 화(火)   3: 토(土)   4: 금(金)   5: 수(水)\n\n";

        bool validInput = false;
        while (!validInput) {
            std::cout << "선택 (1-5 또는 단축키 G/R/Y/P/B, 종료 Q): ";
            if (!std::getline(std::cin, line)) {
                std::cout << "\n입력이 종료되었습니다. 게임을 종료합니다.\n";
                goto finish;
            }
            line = trim(line);
            if (line.empty()) continue;

            // 종료 확인 (단일 문자 Q/q)
            if (line.size() == 1 && (line[0] == 'q' || line[0] == 'Q')) {
                goto finish;
            }

            // 단축키 문자 처리 (단일 문자 알파벳)
            if (line.size() == 1 && std::isalpha(static_cast<unsigned char>(line[0]))) {
                int mapped = shortcutToChoice(line[0]);
                if (mapped == 0) {
                    std::cout << "알 수 없는 단축키입니다. G/R/Y/P/B 또는 1-5를 입력하세요.\n";
                    continue;
                }
                player = mapped;
                validInput = true;
                break;
            }

            // 숫자 입력 처리 (한 자리 또는 여러 자리)
            bool allDigits = true;
            for (char c : line) if (!std::isdigit(static_cast<unsigned char>(c))) { allDigits = false; break; }
            if (allDigits) {
                try {
                    int v = std::stoi(line);
                    if (v < 1 || v > 5) {
                        std::cout << "1~5 숫자만 입력하세요.\n";
                        continue;
                    }
                    player = v;
                    validInput = true;
                    break;
                } catch (const std::exception&) {
                    std::cout << "숫자 변환 오류입니다. 다시 입력하세요.\n";
                    continue;
                }
            }

            // 그 외의 입력
            std::cout << "잘못된 입력입니다. 1-5 숫자, G/R/Y/P/B 단축키, 또는 Q로 종료하세요.\n";
        }

        computer = dist(rng);

        std::cout << "\n\n";
        std::cout << "당신      vs      상대\n\n";

        // 플레이어 원
        printCircle(colorEnabled, getColor(player), getName(player));

        std::cout << "                VS                \n\n";

        // 컴퓨터 원
        printCircle(colorEnabled, getColor(computer), getName(computer));

        // 결과 계산 및 출력
        std::string reason;
        int result = compareWithReason(player, computer, reason);
        if (result == 0) {
            std::cout << "비겼습니다! 오행의 완벽한 조화...\n";
            ++scoreTie;
        } else if (result == 1) {
            std::cout << "승리!!! 당신의 오행이 상대를 제압했습니다!!!\n";
            ++scorePlayer;
        } else {
            std::cout << "패배... 다음엔 더 강한 기운으로 돌아오세요...\n";
            ++scoreComputer;
        }
        std::cout << "- 이유: " << reason << "\n\n";

        // 계속 여부 묻기
        while (true) {
            std::cout << "다시 한 판 하시겠습니까? (y/n, 또는 Q로 종료): ";
            if (!std::getline(std::cin, line)) {
                std::cout << "\n입력이 종료되었습니다. 게임을 종료합니다.\n";
                goto finish;
            }
            line = trim(line);
            if (line.empty()) continue;
            char c = line[0];
            if (c == 'y' || c == 'Y') {
                break; // 다음 라운드
            } else if (c == 'n' || c == 'N' || c == 'q' || c == 'Q') {
                goto finish;
            } else {
                std::cout << "y 또는 n을 입력하세요 (또는 Q로 종료).\n";
            }
        }
    }

finish:
    std::cout << "\n게임 종료 — 최종 스코어\n";
    std::cout << "당신: " << scorePlayer << "   상대: " << scoreComputer << "   무승부: " << scoreTie << "\n";
    if (scorePlayer > scoreComputer) {
        std::cout << "최종 승자: 당신! 오늘의 오행 수련이 훌륭했습니다.\n";
    } else if (scorePlayer < scoreComputer) {
        std::cout << "최종 승자: 상대. 다음에 더 강한 기운으로 도전하세요.\n";
    } else {
        std::cout << "최종: 무승부. 오행의 균형이 유지되었습니다.\n";
    }

    std::cout << "\n오늘도 오행 수련 감사했습니다. 다음에 또 만나요~\n";
    return 0;
}
