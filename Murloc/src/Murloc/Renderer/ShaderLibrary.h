#pragma once

#include "Vulkan/Shader.h"

namespace Murloc
{
	struct ShaderData
	{
		std::string FolderFilePath;
		std::string FilePath;

		size_t PreviouslyHashedSources;
		std::unordered_map<ShaderType, std::string> Sources;
		std::unordered_map<ShaderType, std::vector<uint32_t>> CompiledSources;

		Ref<Shader> ShaderInstance;
	};


	class ShaderLibrary
	{
	public:
		ShaderLibrary() {}
		~ShaderLibrary() {}

		static void Init(const char* filepath) { s_Instance.Init_Impl(filepath); }
		
	   /*
		* Loads or get shaders
		* 
		*/
		static Ref<Shader> LoadOrGet(const char* shaderName);
	private:
		void Init_Impl(const std::string& filepath);
		const ShaderData& LoadOrGet_Impl(const char* shaderName);

		bool CheckUpToDate(ShaderData& shaderData);

		void Compile(ShaderData& shaderData);
		void UpdateYAML(ShaderData& shaderData);

		void Reflect(ShaderType type, const std::vector<uint32_t> compiled, const std::string& filepath);
		void ReadAndCheckIfUpToDate();

		std::string m_ShaderLibraryFilePath;
		//std::unordered_map<Ref<Shader>>

		std::unordered_map<std::string, ShaderData> m_ShaderLibrary;

		std::unordered_map<ShaderType, std::string> m_ShaderSources;

		static ShaderLibrary s_Instance;
	};
}
