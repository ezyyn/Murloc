#pragma once

#include <vulkan/vulkan.h>

namespace PG {

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
		SAMPLERS,
		IMAGES,
		DESCRIPTOR_SET_LAYOUT,
		SHADER_MODULE, 
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

		// TODO: Create faster cleanup command queue
		/*template<class VulkanHandle>
		void PushBackd(VulkanHandle handle) {
			//m_Queue.emplace_back(priority, function);
		}
		template<>
		void PushBackd(VkImage handle) {
			m_ImageDestroyQueue.emplace_back(handle);
		}*/

		template<class F>
		void PushBack(CommandQueueExecutionPriority priority, F&& function) {
			m_Queue.emplace_back(priority, function);
		}
		void Execute();

		size_t Size() const { return m_Queue.size(); }
	private:



		std::vector<Command> m_Queue;
	};

}