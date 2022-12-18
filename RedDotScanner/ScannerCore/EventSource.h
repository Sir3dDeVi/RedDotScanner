#ifndef __EVENTSOURCE_H__
#define __EVENTSOURCE_H__

#include <cassert>

#include <functional>
#include <list>
#include <mutex>


namespace scanner
{
	template <class T>
	class EventSourceT
	{
	protected:
		std::recursive_mutex m_EventHandlersMutex;
		std::list<T*> m_EventHandlers;

		template <class t_func, class... t_params>
		void invoke(t_func func, t_params... params)
		{
			std::lock_guard guard(m_EventHandlersMutex);

			auto handlers = m_EventHandlers; // copy them, because removeEventHandler() could be called from a handler

			for (T* pHandler : handlers)
				std::invoke(func, pHandler, params...);
		}
	public:
		void addEventHandler(T* pHandler)
		{
			std::lock_guard guard(m_EventHandlersMutex);

			m_EventHandlers.push_back(pHandler);
		}

		void removeEventHandler(T* pHandler)
		{
			std::lock_guard guard(m_EventHandlersMutex);

			for (auto it = m_EventHandlers.begin(); it != m_EventHandlers.end(); ++it) {
				if (*it == pHandler) {
					m_EventHandlers.erase(it);
					return;
				}
			}

			assert(false); // Shouldn't get here
		}
	};

}

#endif // __EVENTSOURCE_H__