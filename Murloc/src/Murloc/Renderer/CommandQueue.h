#pragma once

namespace Murloc {

	// In ascending order
	enum CommandQueueExecutionPriority : uint32_t
	{
		FRAMEBUFFERS = 0,
		IMAGEVIEWS,
		SWAPCHAIN,
		PIPELINE,
		PIPELINE_LAYOUT,
		RENDERPASS,
		BUFFER, 
		DESCRIPTOR_POOL,
		DESCRIPTOR_SET_LAYOUT,
		SHADER_MODULE, // NOT SURE
		SYNC_OBJECTS,
		COMMAND_POOLS,
		LOGICAL_DEVICE,
		DEBUG_MESSENGER,
		SURFACE,
		VK_INSTANCE
	};

	struct Command {
		CommandQueueExecutionPriority Priority;
		std::function<void()> Function;

		Command(CommandQueueExecutionPriority priority, const std::function<void()>& function)
			: Priority(priority), Function(function)
		{
		}

		bool operator<(const Command& other) const
		{
			return (Priority < other.Priority);
		}
	};

	class CommandQueue {
	public:
		CommandQueue();
		~CommandQueue();

		CommandQueue(const CommandQueue&) = delete;

		const CommandQueue& operator=(const CommandQueue& other) = delete;

		void PushBack(CommandQueueExecutionPriority priority, const std::function<void()>& function);
		void Execute();

		size_t Size() const { return m_Queue.size(); }
	private:
		std::list<Command> m_Queue;
	};

}