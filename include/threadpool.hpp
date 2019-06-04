#pragma once
#include <iostream>
#include <mutex>
#include <functional>
#include <queue>
#include <thread>
#include <condition_variable>
#include <map>
#include <sys/time.h>

namespace threadpool
{

class fixed_thread_pool
{
public:
	explicit fixed_thread_pool(size_t thread_count, size_t timer_count)
		: data_(std::make_shared<data>())
		, timer_(std::make_shared<timer>()) {
		for (size_t i = 0; i < thread_count; ++i) {
			if (data_ == 0) {
				std::cout << "all 0x00000" << std::endl;
				return;
			}
			std::thread([data = data_] {
			/*std::thread([&] {
				auto data = data_;*/
				std::unique_lock<std::mutex> lk(data->mtx_);
				for (;;) {
					while (!data->is_start_) {
						data->is_start_cond_.wait(lk);
					}
					if (!data->tasks_.empty()) {
						auto current = std::move(data->tasks_.front());//左值转右值,更替所有权
						//auto current = data->tasks_.front();
						data->tasks_.pop();
						lk.unlock();
						current();
						lk.lock();
					} else if (data->is_shutdown_) {
						break;
					} else {
						data->is_empty_cond_.notify_one();//xz
						data->cond_.wait(lk);
					}
				}
			}).detach();
		}
		for (size_t i = 0; i < timer_count; ++i) {
			std::thread([timer = timer_](void){
				std::unique_lock<std::mutex> lk(timer->mtx_);
				for (;;) {
					if (!timer->tasks_.empty()) {
						auto task = timer->tasks_.begin();//左值转右值,更替所有权
						auto info = task->first;
						uint64_t now_ms = get_current_ms();
						if (now_ms < info.clock_ms) {//时间未到
							lk.unlock();
							std::this_thread::sleep_for(std::chrono::milliseconds(info.clock_ms - now_ms));
							lk.lock();
							continue;
						}
						auto current = std::move(task->second);
						if (info.is_cycle) {//重装定时器
							info.clock_ms = info.interval_ms + get_current_ms();
							timer->tasks_.emplace(info, current);
						}
						timer->tasks_.erase(task);
						lk.unlock();
						current();
						lk.lock();
					}
					else if (timer->is_shutdown_) {
						break;
					}
					else {
						timer->cond_.wait(lk);
					}
				}
			}).detach();
		}
	}

	fixed_thread_pool() = default;
	fixed_thread_pool(fixed_thread_pool&&) = default;

	~fixed_thread_pool() {
		if ((bool) data_) {
			{
				std::lock_guard<std::mutex> lk(data_->mtx_);
				data_->is_shutdown_ = true;
			}
			data_->cond_.notify_all();
		}

		if ((bool)timer_) {
			{
				std::lock_guard<std::mutex> lk(timer_->mtx_);
				timer_->is_shutdown_ = true;
			}
			timer_->cond_.notify_all();
		}
	}

	void start(void) {
		std::lock_guard<std::mutex> lk(data_->mtx_);
		data_->is_start_ = true;
		data_->is_start_cond_.notify_all();
	}
	 
	void wait_empty() {
		std::unique_lock<std::mutex> lk(data_->mtx_);
		data_->is_empty_cond_.wait(lk, [&]{return data_->tasks_.empty();});//条件满足则继续执行
		lk.unlock();
	}
	
template <class F>
void execute(F&& task) {
	{
		std::lock_guard<std::mutex> lk(data_->mtx_);
		data_->tasks_.emplace(std::forward<F>(task));
	}
	data_->cond_.notify_one();
}

template <class F>
void execute(F&& task, std::string name, uint64_t interval_ms, bool is_cycle = true) {
	{
		info info_;
		info_.name = name;
		info_.interval_ms = interval_ms;
		info_.clock_ms = interval_ms + get_current_ms();
		info_.is_cycle = is_cycle;

		std::lock_guard<std::mutex> lk(timer_->mtx_);
		timer_->tasks_.emplace(std::forward<info>(info_),std::forward<F>(task));
	}
	timer_->cond_.notify_one();
}

static uint64_t get_current_ms()
{
	timeval tv;
	gettimeofday(&tv, 0);
	unsigned long long ret = tv.tv_sec;
	return ret * 1000 + tv.tv_usec / 1000;
}

private:
	struct data {
        std::mutex mtx_;
        std::condition_variable cond_;
        bool is_shutdown_ = false;
        std::queue<std::function<void()>> tasks_;

		bool is_start_ = false;
		std::condition_variable is_start_cond_;

		std::condition_variable is_empty_cond_;

    };

	struct info {
		std::string name = "";
		uint64_t interval_ms;
		uint64_t clock_ms;
		bool is_cycle;
		bool operator<(const info& other) const {
			return clock_ms < other.clock_ms;
		}
		bool operator>(const info& other) const {
			return clock_ms > other.clock_ms;
		}
	};
	struct timer {
		std::mutex mtx_;
		std::condition_variable cond_;
		bool is_shutdown_ = false;
		std::multimap<info,std::function<void()>> tasks_;
		bool is_start_ = false;
	};
	std::shared_ptr<data> data_;
	std::shared_ptr<timer> timer_;
};
}
