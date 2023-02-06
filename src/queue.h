#pragma once

#include "types.h"

#include <queue>
#include <mutex>

namespace cpurt
{
	template <typename T>
	class BlockingQueue
	{
	public:
		BlockingQueue() = default;
		~BlockingQueue() = default;

		void push(const T &v)
		{
			{
				std::scoped_lock lock{m_mutex};
				m_queue.push(v);
			}

			m_signal.notify_one();
		}

		void push(T &&v)
		{
			{
				std::scoped_lock lock{m_mutex};
				m_queue.push(v);
			}

			m_signal.notify_one();
		}

		T wait()
		{
			std::unique_lock lock{m_mutex};

			while (m_queue.empty())
			{
				m_signal.wait(lock);
			}

			auto v = m_queue.front();
			m_queue.pop();
			return v;
		}

	private:
		std::queue<T> m_queue{};

		std::mutex m_mutex{};
		std::condition_variable m_signal{};
	};
}
