#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#define MAX_USERNAME_LENGTH 30
#define MAX_PASSWORD_LENGTH 30
#define MAX_SCORES 100  // 假设每个文件最多有100个得分

#define CREDENTIALS_FILE "credentials.txt"
#define EASY_HIGHSCORES_FILE "highscores_easy.txt"
#define MEDIUM_HIGHSCORES_FILE "highscores_medium.txt"
#define HARD_HIGHSCORES_FILE "highscores_hard.txt"
#define RECORDS_FILE "user_records.txt"

// 保存用户名和密码到文件
void save_credentials(const char *username, const char *password) {
    FILE *file = fopen(CREDENTIALS_FILE, "a");
    if (file == NULL) {
        perror("Failed to open credentials file");
        exit(1);
    }
    fprintf(file, "%s:%s\n", username, password);
    fclose(file);
}

// 验证用户名和密码
int validate_credentials(const char *username, const char *password) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        perror("Failed to open credentials file");
        return 0; // 文件不存在，验证失败
    }

    char line[256];
    char stored_username[100];
    char stored_password[100];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%[^:]:%s", stored_username, stored_password) == 2) {
            if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0) {
                fclose(file);
                return 1; // 验证成功
            }
        }
    }

    fclose(file);
    return 0; // 验证失败
}

void center_text(WINDOW *win, int y, const char *text) {
    int x = (COLS - strlen(text)) / 2;
    mvwprintw(win, y, x, "%s", text);
}

// 读取文件内容到整型数组
void read_scores_to_array(const std::string& file_path, int* scores_array) {
    std::ifstream file(file_path);
    int index = 0; // 声明index变量

    if (file.is_open()) {
        std::string line;
        while (getline(file, line) && index < MAX_SCORES) {
            scores_array[index++] = std::stoi(line);
        }
        file.close();
    }
    // 初始化剩余位置为0
    for (int i = index; i < MAX_SCORES; ++i) {
        scores_array[i] = 0;
    }
}

// 比较两个数组并找出变化的内容
void find_differences(int* array1, int* array2, int* result_array) {
    for (int i = 0; i < MAX_SCORES; ++i) {
        if (array1[i] != array2[i]) {
            result_array[i] = array2[i]; // 只记录变化的内容
        } else {
            result_array[i] = 0; // 没有变化的内容记录为0
        }
    }
}

// 记录变化的得分到文件
void record_changed_scores(const std::string& username, const std::string& mode, int* scores_array) {
    std::ofstream file(RECORDS_FILE, std::ios::app);
    if (file.is_open()) {
        for (int i = 0; i < MAX_SCORES; ++i) {
            if (scores_array[i] != 0) {
                file << username << ";" << mode << ";" << scores_array[i] << std::endl;
            }
            if (scores_array[i] == 0){
                if (scores_array[i++] != 0){
                    file << username << ";" << mode << ";" << scores_array[i] << std::endl;
                }
            }
                else {
                break;  // 假设0之后没有有效数据
            }
        }
        file.close();
    }
}

/*
// 检测文件修改并记录变化的得分
void detect_and_log_score_changes(const std::string& username) {



}
*/

// main函数
int main() {
    WINDOW *login_win;
    int ch;
    char username[MAX_USERNAME_LENGTH + 1] = {0};
    char password[MAX_PASSWORD_LENGTH + 1] = {0};
    int username_pos = 0;
    int password_pos = 0;
    int current_field = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();

    login_win = newwin(10, 45, (LINES - 10) / 2, (COLS - 45) / 2);
    box(login_win, 0, 0);

    center_text(login_win, 1, "Login to System");

    mvwprintw(login_win, 3, 5, "Username:");
    mvwprintw(login_win, 4, 5, "Password:");

    mvwprintw(login_win, 3, 15, "_______________________");
    mvwprintw(login_win, 4, 15, "_______________________");

    wmove(login_win, 3, 15);

    wrefresh(login_win);

    while (1) {
        ch = wgetch(login_win);

        if (current_field == 0) {
            if (ch == '\n') {
                current_field = 1;
                mvwprintw(login_win, 4, 15, "_______________________");
                wmove(login_win, 4, 15);
                wrefresh(login_win);
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (username_pos > 0) {
                    username[--username_pos] = '\0';
                    mvwprintw(login_win, 3, 15 + username_pos, " ");
                }
            } else if (ch >= 32 && ch <= 126 && username_pos < MAX_USERNAME_LENGTH) {
                username[username_pos++] = ch;
                mvwprintw(login_win, 3, 15 + username_pos - 1, "%c", ch);
            }
        } else if (current_field == 1) {
            if (ch == '\n') {
                break;
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (password_pos > 0) {
                    password[--password_pos] = '\0';
                    mvwprintw(login_win, 4, 15 + password_pos, " ");
                }
            } else if (ch >= 32 && ch <= 126 && password_pos < MAX_PASSWORD_LENGTH) {
                password[password_pos++] = ch;
                mvwprintw(login_win, 4, 15 + password_pos - 1, "*");
            }
        }

        wrefresh(login_win);
    }


    if (validate_credentials(username, password)) {
        mvprintw(LINES - 2, 0, "Login successful!");
        refresh();
        getch();

        delwin(login_win);

 //       detect_and_log_score_changes(username);

        endwin();

            // 定义三个模式的文件路径
    const std::string easy_file = EASY_HIGHSCORES_FILE;
    const std::string medium_file = MEDIUM_HIGHSCORES_FILE;
    const std::string hard_file = HARD_HIGHSCORES_FILE;

    // 定义数组
    int easy_scores1[MAX_SCORES] = {0};
    int medium_scores1[MAX_SCORES] = {0};
    int hard_scores1[MAX_SCORES] = {0};

    int easy_scores2[MAX_SCORES] = {0};
    int medium_scores2[MAX_SCORES] = {0};
    int hard_scores2[MAX_SCORES] = {0};

    int easy_diff[MAX_SCORES] = {0};
    int medium_diff[MAX_SCORES] = {0};
    int hard_diff[MAX_SCORES] = {0};

    // 第一次读取文件
    read_scores_to_array(easy_file, easy_scores1);
    read_scores_to_array(medium_file, medium_scores1);
    read_scores_to_array(hard_file, hard_scores1);

    // 启动游戏程序
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(
            "gtk_app.exe",
            NULL,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            mvprintw(LINES - 2, 0, "Failed to start program. Error code: %d", GetLastError());
        } else {
            mvprintw(LINES - 2, 0, "Program started successfully.");
        }


    WaitForSingleObject(pi.hProcess, INFINITE);

    // 第二次读取文件
    read_scores_to_array(easy_file, easy_scores2);
    read_scores_to_array(medium_file, medium_scores2);
    read_scores_to_array(hard_file, hard_scores2);

    // 找出变化
    find_differences(easy_scores1, easy_scores2, easy_diff);
    find_differences(medium_scores1, medium_scores2, medium_diff);
    find_differences(hard_scores1, hard_scores2, hard_diff);

    // 记录变化
    if (std::any_of(std::begin(easy_diff), std::end(easy_diff), [](int val) { return val != 0; })) {
        record_changed_scores(username, "简单模式", easy_diff);
    }
    if (std::any_of(std::begin(medium_diff), std::end(medium_diff), [](int val) { return val != 0; })) {
        record_changed_scores(username, "普通模式", medium_diff);
    }
    if (std::any_of(std::begin(hard_diff), std::end(hard_diff), [](int val) { return val != 0; })) {
        record_changed_scores(username, "困难模式", hard_diff);
    }

    // 清理
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    } else {
        mvprintw(LINES - 2, 0, "Login failed. Invalid username or password.");
        refresh();
        getch();

        delwin(login_win);
        endwin();

        return 0;
    }

    return 0;
}
