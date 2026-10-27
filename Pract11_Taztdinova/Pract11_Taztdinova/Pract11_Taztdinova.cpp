#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>

using namespace std;

const int MAX_ROBOTS = 10;
const int MAX_TESTS = 100;

struct Robot {
    unsigned int id;
    unsigned int energy;
    unsigned int score;
    bool is_finished;
    HANDLE hThread;
};

Robot robots[MAX_ROBOTS];
int test_program[MAX_TESTS];
int test_count = 0;
int num_robots = 0;

DWORD WINAPI robot_work(LPVOID lpParam) {
    int index = (int)lpParam;
    Robot& r = robots[index];
    srand((unsigned int)time(NULL) + index * 17);

    cout << "\nРобот " << r.id << ": начал испытания." << endl;

    for (int i = 0; i < test_count; i++) {
        int test_code = test_program[i];
        const char* test_name = "";
        int time = 0;
        int energy_cost = 0;
        int bonus = 0;

        if (rand() % 10 < 2) {
            cout << "\nРобот " << r.id << ": сломался!" << endl;
            Sleep(2000);
            r.energy -= 10;
            if (r.energy < 0) {
                r.energy = 0;
            }
            cout << "\nРобот " << r.id << ": ремонт закончен. Энергия: " << r.energy << endl;

            Sleep((1 + rand() % 3) * 1000);
            continue;
        }

        if (test_code == 1){ 
            test_name = "Препятствие"; 
            time = 3; 
            energy_cost = 10; 
            bonus = 15; 
        }
        else if (test_code == 2) { 
            test_name = "Спринт"; 
            time = 4; 
            energy_cost = 20; 
            bonus = 30; 
        }
        else if (test_code == 4) {
            test_name = "Марафон";
            time = 10; 
            energy_cost = 60; 
            bonus = 55; 
        }
        else if (test_code == 3) { 
            test_name = "Подзарядка"; 
            time = 1 + (rand() % 5); 
        }

        if (test_code != 3 && r.energy < energy_cost) {
            cout << "\nРобот " << r.id << ": пропускает " << test_name << " (мало энергии)." << endl;
            Sleep((1 + rand() % 3) * 1000);
            continue;
        }

        cout << "\nРобот " << r.id << ": начал " << test_name << " (" << time << " сек)" << endl;

        for (int j = 0; j < time; j++) {
            Sleep(1000);
        }

        if (test_code == 3) {
            r.energy += time * 10;
            if (r.energy > 100) {
                r.energy = 100;
            }
            cout << "\nРобот " << r.id << ": зарядился. Энергия: " << r.energy << endl;
        }
        else {
            r.energy -= energy_cost;
            r.score += bonus;
            cout << "\nРобот " << r.id << ": завершил. Энергия: " << r.energy << endl;
        }

        Sleep((1 + rand() % 3) * 1000);
    }

    r.is_finished = true;
    cout << "\nРобот " << r.id << ": завершил. Очки: " << r.score << endl;
    return 0;
}

bool load_tests() {
    ifstream file("arena.dat", ios::binary);
    if (!file.is_open()) {
        cout << "\nФайл arena.dat не найден!" << endl;
        return false;
    }

    int val;
    test_count = 0;
    while (file.read((char*)&val, sizeof(int)) && test_count < MAX_TESTS) {
        test_program[test_count] = val;
        test_count++;
    }
    cout << "\nЗагружено испытаний: " << test_count << endl;
    return true;
}

int main() {
    setlocale(NULL, "Russian");
    srand(time(0));

    cout << "\nВведите сколько роботов (от 3 до 9)" << endl;
    cin >> num_robots;
    if (num_robots < 3 || num_robots > 10) {
        cout << "\nВы ввели неправильное колличество роботов" << endl;
    }

    if (!load_tests()) {
        return 1;
    }

    for (int i = 0; i < num_robots; i++) {
        robots[i].id = i + 1;
        robots[i].energy = 60;
        robots[i].score = 0;
        robots[i].is_finished = false;
        robots[i].hThread = CreateThread(NULL, 0, robot_work, (LPVOID)i, 0, NULL);
    }

    cout << "\nАрена" << endl;
    cout << "\nНажмите от 1 до " << num_robots << " для паузы/возобновления." << endl;

    static bool paused[MAX_ROBOTS] = { false };

    while (true) {
        bool all_finished = true;
        for (int i = 0; i < num_robots; i++) {
            if (!robots[i].is_finished) all_finished = false;
        }
        if (all_finished) {
            break;
        }

        if (_kbhit()) {
            char key = _getch();
            if (key >= '1' && key <= '9') {
                int index = key - '1';
                if (index >= 0 && index < num_robots && !robots[index].is_finished) {
                    if (!paused[index]) {
                        SuspendThread(robots[index].hThread);
                        paused[index] = true;
                        cout << "\nРобот " << robots[index].id << " остановлен." << endl;
                    }
                    else {
                        ResumeThread(robots[index].hThread);
                        paused[index] = false;
                        cout << "\nРобот " << robots[index].id << " востановлен." << endl;
                    }
                }
            }
        }

        Sleep(50);
    }

    cout << "\nВсе роботы закончили!" << endl;

    for (int i = 0; i < num_robots; i++) {
        WaitForSingleObject(robots[i].hThread, INFINITE);
        CloseHandle(robots[i].hThread);
    }

    for (int i = 0; i < num_robots - 1; i++) {
        for (int k = 0; k < num_robots - 1 - i; k++) {
            if (robots[k].score < robots[k + 1].score) {
                Robot temp = robots[k];
                robots[k].id = robots[k + 1].id;
                robots[k].energy = robots[k + 1].energy;
                robots[k].score = robots[k + 1].score;
                robots[k + 1].id = temp.id;
                robots[k + 1].energy = temp.energy;
                robots[k + 1].score = temp.score;
            }
        }
    }

    cout << "\nРезультаты" << endl;
    for (int i = 0; i < num_robots; i++) {
        cout << "Робот " << robots[i].id << ": " << robots[i].score << " очков" << endl;
    }

    if (num_robots > 1 && robots[0].score == robots[1].score) {
        cout << "\nНичья!" << endl;
    }
    else {
        cout << "\nПобедитель: Робот " << robots[0].id << "!" << endl;
    }

    _getch();

    return 0;
}