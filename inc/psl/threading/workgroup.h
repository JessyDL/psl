#pragma once
#include <thread>
#include <future>
#include "psl/array.h"
#include "psl/dequeue.h"
#include "psl/spmc.h"

namespace psl::threading
{
	class task_base
	{
	  public:
		virtual ~task_base()	  = default;
		virtual void operator()() = 0;
	};

	template <typename R, typename... Args>
	class task : public task_base
	{
	  public:
		task() = default;
		virtual ~task(){};

		template <typename... Params>
		task(std::packaged_task<R(Args...)>&& task, [[maybe_unused]] Params&&... args) : m_Task(std::move(task))
		{
			m_Fun = [... args = std::forward<Params>(args)](std::packaged_task<R(Args...)>&& t) mutable {
				std::invoke(t, std::forward<Params>(args)...);
			};
		}

		void operator()() override { std::invoke(m_Fun, std::move(m_Task)); }

	  private:
		std::packaged_task<R(Args...)> m_Task;
		std::function<void(std::packaged_task<R(Args...)>&&)> m_Fun{};
	};

	class thread
	{
	  public:
		thread(std::function<void()> setup = {}, std::function<void()> teardown = {}) noexcept
			: m_Thread([setup, teardown, &idle = this->m_Idle, &taskpool = this->m_TaskPool,
						&cv = this->m_ConditionVariable, &paused = this->m_Paused, &mutex = this->m_Mutex,
						&thread_paused = this->m_ThreadPaused](std::stop_token token) mutable {
				  if(setup) setup();
				  size_t idle_counter = 0;
				  while(!token.stop_requested())
				  {
					  if(paused)
					  {
						  std::unique_lock<std::mutex> lk(mutex);
						  idle			= true;
						  idle_counter  = 100;
						  thread_paused = true;
						  cv.wait(lk);
						  thread_paused = false;
						  lk.unlock();
					  }
					  ++idle_counter;
					  for(auto& consumer : taskpool)
					  {
						  auto task = consumer.pop();
						  if(task)
						  {
							  idle_counter = 0;
							  std::invoke(*task.value().get());
							  break;
						  }
					  }
					  idle = idle_counter > 100;
					  if(idle_counter > 100)
					  {
						  idle = true;
					  }
				  }
				  if(teardown) teardown();
			  })
		{}
		~thread() noexcept
		{
			if(m_Thread.joinable())
			{
				m_Thread.request_stop();
				m_Thread.join();
			}
		};
		thread(const thread& other) noexcept = delete;
		thread(thread&& other) noexcept		 = default;
		thread& operator=(const thread& other) noexcept = delete;
		thread& operator=(thread&& other) noexcept = default;

		bool idle() const noexcept { return m_Idle; }
		bool paused() const noexcept { return m_Paused; }

		void request_stop() { m_Thread.request_stop(); }

		void pause(bool wait_for_idle = false)
		{
			if(m_Paused) return;
			{
				auto lk  = lock();
				m_Paused = true;
			}
			if(wait_for_idle)
			{
				while(!m_ThreadPaused)
				{
				}
			}
		}

		void resume()
		{
			if(!m_Paused) return;
			auto lk  = lock();
			m_Paused = false;
			notify();
		}

		void add(psl::spmc::consumer<std::shared_ptr<task_base>> task_pool)
		{
			auto was_paused = paused();
			if(!was_paused) pause(true);
			m_TaskPool.emplace_back(task_pool);
			if(!was_paused) resume();
		}

		void remove(psl::spmc::consumer<std::shared_ptr<task_base>> task_pool)
		{
			auto was_paused = paused();
			if(!was_paused) pause(true);
			m_TaskPool.erase(std::find(std::begin(m_TaskPool), std::end(m_TaskPool), task_pool), std::end(m_TaskPool));
			if(!was_paused) resume();
		}


	  private:
		[[nodiscard]] inline std::lock_guard<std::mutex> lock() { return std::lock_guard<std::mutex>{m_Mutex}; }
		inline void notify() { m_ConditionVariable.notify_one(); }

		bool m_ThreadPaused{false};
		bool m_Paused{false};
		std::condition_variable m_ConditionVariable;
		std::mutex m_Mutex;
		bool m_Idle{true};
		psl::array<psl::spmc::consumer<std::shared_ptr<task_base>>> m_TaskPool;
		std::jthread m_Thread;
	};

	class workgroup
	{
	  public:
		workgroup(std::span<std::shared_ptr<thread>> threads = {}) noexcept : m_Threads(threads)
		{
			for(auto& thread : m_Threads) thread->add(m_Tasks.consumer());
		};
		~workgroup() noexcept {};
		workgroup(const workgroup& other) noexcept = default;
		workgroup(workgroup&& other) noexcept	  = default;
		workgroup& operator=(const workgroup& other) noexcept = default;
		workgroup& operator=(workgroup&& other) noexcept = default;

		template <class R, class... Args, typename... Params>
		auto operator()(std::packaged_task<R(Args...)>&& packaged_task, Params&&... args)
		{
			m_Tasks.push(std::make_shared<task<R, Args...>>(std::move(packaged_task), std::forward<Params>(args)...));
		}

		bool all_threads_idle() const noexcept
		{
			return std::all_of(std::begin(m_Threads), std::end(m_Threads), [](auto& thread) { return thread->idle(); });
		}

		void add(std::shared_ptr<thread> thread)
		{
			m_Threads.emplace_back(std::move(thread));
			m_Threads.back()->add(m_Tasks.consumer());
		}

		void remove(std::shared_ptr<thread>& thread)
		{
			thread->remove(m_Tasks.consumer());
			m_Threads.erase(std::find(std::begin(m_Threads), std::end(m_Threads), thread));
		}

	  private:
		psl::spmc::producer<std::shared_ptr<task_base>> m_Tasks{};
		psl::array<std::shared_ptr<thread>> m_Threads{};
	};
} // namespace psl::threading