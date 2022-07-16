#include "murpch.hpp"

#include "CommandQueue.h"

namespace Murloc {

	CommandQueue::CommandQueue()
	{
	}

	CommandQueue::~CommandQueue()
	{
	}

	void CommandQueue::PushBack(CommandQueueExecutionPriority priority, const std::function<void()>& function)
	{
		m_Queue.emplace_back(priority, function);
	}

	void CommandQueue::Execute()
	{
		m_Queue.sort();

		for (auto& command : m_Queue)
			command.Function();
		m_Queue.clear();
	}
}