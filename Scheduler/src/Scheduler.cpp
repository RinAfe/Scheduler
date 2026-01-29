#include "Scheduler.h"

Scheduler::Scheduler() : stop_flag(false), worker_thread(&Scheduler::workerFunction, this) {}

Scheduler::~Scheduler() {

	stop_flag.store(true);

	{
		std::lock_guard<std::mutex> lock(queue_mutex);
		condition.notify_all();
	}
	
	if (worker_thread.joinable()) {
		try {
			worker_thread.join();
		}
		catch (const std::system_error& e) {
			std::cerr << "std::system_error: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "std::exception: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown std::exception" << std::endl;
		}
	}

}

void Scheduler::scheduleAfter(std::chrono::milliseconds delay, std::function<void()> task) {

	auto scheduled_time = std::chrono::steady_clock::now() + delay;

	scheduleAt(scheduled_time, std::move(task));
}

void Scheduler::scheduleAt(std::chrono::steady_clock::time_point time,
	std::function<void()> task) {
	if (stop_flag.load()) {
		throw std::runtime_error("Scheduler is stopped");
	}

	ScheduledTask new_task;
	new_task.time = time;  // ��� ���������!
	new_task.task = std::move(task);

	{
		std::lock_guard<std::mutex> lock(queue_mutex);
		tasks.push(std::move(new_task));
	}

	condition.notify_one();
}

void Scheduler::workerFunction()
{
	using namespace std::chrono;

	while (!stop_flag.load()) {
		std::unique_lock<std::mutex> lock(queue_mutex);

		if (tasks.empty()) {
			condition.wait(lock);
		} else {
			condition.wait_until(lock, tasks.top().time);
		}

		if (tasks.empty()) {
			continue;
		}

		if (steady_clock::now() < tasks.top().time) {
			continue;
		}

		const auto [_, task] = tasks.top();
		tasks.pop();

		lock.unlock();

		try {
			task();
		} catch (const std::exception &e) {
			std::cerr << "Scheduler task error: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Scheduler unknown task error" << std::endl;
		}
	}
}

void Scheduler::stop() {
	bool except = false;
	if (stop_flag.compare_exchange_strong(except, true)) {
		condition.notify_all();
	}
}

bool Scheduler::empty() const {
	std::lock_guard<std::mutex> lock(queue_mutex);
	return tasks.empty();
}

size_t Scheduler::size() const {
	std::lock_guard<std::mutex> lock(queue_mutex);
	return tasks.size();
}