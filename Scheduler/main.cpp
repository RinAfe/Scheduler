#include "Scheduler.h"

int main() {
    Scheduler scheduler;

    scheduler.scheduleAfter(std::chrono::seconds(1), []() {
        std::cout << "Task 1 executed after 1s" << std::endl;
        });

    scheduler.scheduleAfter(std::chrono::seconds(2), []() {
        std::cout << "Task 2 executed after 2s" << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}