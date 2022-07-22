#include "pgpch.h"

#include "CommandQueue.h"

namespace PG {

	CommandQueue::CommandQueue()
	{
	}

	CommandQueue::~CommandQueue()
	{
	}

	void CommandQueue::Execute()
	{
		std::sort(m_Queue.begin(), m_Queue.end());

		for (auto& command : m_Queue)
			command.Function();
		m_Queue.clear();
	}
}