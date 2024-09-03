
#ifndef SESSIONS_ID_MANAGER_H
#define SESSIONS_ID_MANAGER_H

#include <stack>
#include <unordered_set>
#include <optional>

class SessionIdManager
{
private:
	std::stack<std::size_t> m_availableIDs;
	std::unordered_set<std::size_t> m_usedIDs;
	std::size_t m_maxSessionIDs;

public:
	SessionIdManager(std::size_t maxIDs) : m_maxSessionIDs(maxIDs)
	{
		for (std::size_t i = 1; i <= m_maxSessionIDs; ++i)
		{
			m_availableIDs.push(i);
		}
	}

	std::optional<std::size_t> getNewSessionID()
	{
		if (m_availableIDs.empty())
		{
			return std::nullopt;
		}

		const std::size_t newID = m_availableIDs.top();
		m_availableIDs.pop();
		m_usedIDs.insert(newID);

		return newID;
	}

	void releaseSessionID(std::size_t id)
	{
		auto it = m_usedIDs.find(id);
		if (it != m_usedIDs.end())
		{
			m_usedIDs.erase(it);
			m_availableIDs.push(id);
		}
	}
};

#endif