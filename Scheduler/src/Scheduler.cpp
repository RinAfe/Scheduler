#include "Scheduler.h"

Scheduler::Scheduler() : stop_flag(false), worker_thread(&Scheduler::workerFunction, this) {}

Scheduler::~Scheduler() {
	stop_flag.store(true);
	condition.notify_all();
	
	try
	{
		if (worker_thread.joinable())
			worker_thread.join();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error join(): " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Error not join(): " << std::endl;
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
	new_task.time = time;  // Уже вычислено!
	new_task.task = std::move(task);

	{
		std::lock_guard<std::mutex> lock(queue_mutex);
		tasks.push(std::move(new_task));
	}

	condition.notify_one();
}

void Scheduler::workerFunction() {
	while (!stop_flag.load()) {
		std::unique_lock<std::mutex> lock(queue_mutex);

		condition.wait(lock, [this]() {
			return stop_flag.load() || !tasks.empty();
			});

		if (stop_flag.load()) return;

		const ScheduledTask& next_task = tasks.top();
		auto now = std::chrono::steady_clock::now();

		if (next_task.time <= now) {
			ScheduledTask task_to_execute = std::move(const_cast<ScheduledTask&>(tasks.top()));
			tasks.pop();
			lock.unlock();

			try {
				task_to_execute.task();
			}
			catch (const std::exception& e) {
				std::cerr << "Scheduler task error: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Scheduler unknown task error" << std::endl;
			}
		}
		else {
			condition.wait_until(lock, next_task.time);
		}
	}
}

void Scheduler::stop() {
	bool except = false;
	if (stop_flag.compare_exchange_strong(except, true)) {
		condition.notify_all();
	}
}

void Scheduler::wait() {
	stop();

	std::call_once(join_once_flag, [this]() {
		if (worker_thread.joinable()) {
			worker_thread.join();
		}
		});
}

bool Scheduler::empty() const {
	std::lock_guard<std::mutex> lock(queue_mutex);
	return tasks.empty();
}

size_t Scheduler::size() const {
	std::lock_guard<std::mutex> lock(queue_mutex);
	return tasks.size();
}