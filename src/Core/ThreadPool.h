#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <future>
#include <queue>
#include <mutex>
#include <vector>
#include "Logger.h"

namespace Frost
{
	class ThreadPool
	{
	public:
		ThreadPool(size_t threadCount = 0)
		{
			if (threadCount == 0)
				threadCount = std::max(1u,
					std::thread::hardware_concurrency() - 1);

			m_Running = true;

			for (size_t i = 0; i < threadCount; i++)
			{
				m_Workers.emplace_back([this]()
					{
						// Worker loop — runs until shutdown
						while (true)
						{
							std::function<void()> job;
							{
								std::unique_lock<std::mutex> lock(m_Mutex);

								// Wait until job available or shutdown
								m_Condition.wait(lock, [this]()
									{
										return !m_Jobs.empty() || !m_Running;
									});

								if (!m_Running && m_Jobs.empty())
									return; // shutdown

								job = std::move(m_Jobs.front());
								m_Jobs.pop();
							}

							job(); // execute outside lock
							m_ActiveJobs--;
							m_FinishedCondition.notify_all();
						}
					});
			}

			FROST_LOG("ThreadPool initialized with %zu workers", threadCount);
		}

		~ThreadPool()
		{
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_Running = false;
			}
			m_Condition.notify_all();
			for (auto& worker : m_Workers)
			{
				if (worker.joinable())
					worker.join();
			}
		}

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		template<typename F, typename... Args>
		auto Submit(F&& func, Args&&... args)
			-> std::future<std::invoke_result_t<F, Args...>>
		{
			using ReturnType = std::invoke_result_t<F, Args...>;

			auto task = std::make_shared<std::packaged_task<ReturnType()>>(
				std::bind(std::forward<F>(func), std::forward<Args>(args)...)
			);

			std::future<ReturnType> future = task->get_future();

			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_Jobs.push([task]() { (*task)(); });
				m_ActiveJobs++;
			}

			m_Condition.notify_one();
			return future;
		}

		void SubmitVoid(std::function<void()> job)
		{
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_Jobs.push(std::move(job));
				m_ActiveJobs++;
			}
			m_Condition.notify_one();
		}

		void WaitAll()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_FinishedCondition.wait(lock, [this]()
				{
					return m_ActiveJobs == 0 && m_Jobs.empty();
				});
		}

		size_t ThreadCount() const
		{
			return m_Workers.size();
		}
		size_t PendingJobs() const
		{
			return m_Jobs.size();
		}

	private:
		int threadCount;
		std::vector<std::thread> m_Workers;
		std::queue<std::function<void()>> m_Jobs;
		std::mutex m_Mutex;
		std::condition_variable m_Condition;
		std::condition_variable m_FinishedCondition;
		std::atomic<int> m_ActiveJobs = { 0 };
		bool m_Running = false;
	};
}