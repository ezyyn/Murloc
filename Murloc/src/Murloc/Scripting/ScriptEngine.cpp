#include "murpch.hpp"

#include "ScriptEngine.h"

#include <fstream>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/class.h>

namespace Murloc {

	namespace Utils {
		static char* ReadBytes(const std::string& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				printf("%s.%s\n", nameSpace, name);
			}
		}
	}

	struct ScriptData {
		MonoDomain* RootDomain;
		MonoDomain* AppDomain;
		MonoAssembly* CoreAssembly;
	};

	static MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = Utils::ReadBytes(assemblyPath, &fileSize);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			// Log some error message using the errorMessage data
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		// Don't forget to free the file data
		delete[] fileData;

		return assembly;
	}

	static ScriptData* s_Data{ nullptr };

	void ScriptEngine::Init()
	{
		s_Data = new ScriptData;

		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("MurlocJITRuntime");
		MUR_CORE_ASSERT(rootDomain);

		// Store the root domain pointer
		s_Data->RootDomain = rootDomain;

		// Create an App Domain
		s_Data->AppDomain = mono_domain_create_appdomain("MyAppDomain", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadCSharpAssembly("assets/scripts/Murloc-ScriptCore.dll");
		Utils::PrintAssemblyTypes(s_Data->CoreAssembly);

		// Gets image of the assembly
		MonoImage* image = mono_assembly_get_image(s_Data->CoreAssembly);
		// Get a reference to the class we want to instantiate
		MonoClass* klass = mono_class_from_name(image, "Murloc", "Main");
		MUR_CORE_ASSERT(klass);
		// Allocates new object
		MonoObject* klassInstance = mono_object_new(s_Data->AppDomain, klass);
		MUR_CORE_ASSERT(klassInstance)
		// Call the parameterless (default) constructor
		mono_runtime_object_init(klassInstance);

		// Get a reference to the method in the class
		MonoMethod* method = mono_class_get_method_from_name(klass, "PrintMessage", 0);
		mono_runtime_invoke(method, klassInstance, nullptr, nullptr);

		{
			int value = 5;
			void* params[1]{
				&value
			};
			MonoMethod* methodPrintInt = mono_class_get_method_from_name(klass, "PrintInt", 1);
			mono_runtime_invoke(methodPrintInt, klassInstance, params, nullptr);
		}
		{
			MonoString* monoString = mono_string_new(s_Data->AppDomain, "Hello World from C++");
			void* params[1]{
				monoString
			};
			MonoMethod* methodPrintString = mono_class_get_method_from_name(klass, "PrintString", 1);
			mono_runtime_invoke(methodPrintString, klassInstance, params, nullptr);
		}

	}

	void ScriptEngine::Shutdown()
	{
		//mono_domain_set(s_Data->RootDomain, true);
		//
		//mono_domain_unload(s_Data->AppDomain);
		//mono_domain_unload(s_Data->RootDomain);
		//mono_runtime_set_shutting_down();
		//
		////mono_domain_finalize(mono_get_root_domain(), 0);
		////mono_runtime_cleanup(mono_get_root_domain());
		//mono_gc_collect(mono_gc_max_generation());
		
		mono_jit_cleanup(s_Data->RootDomain);
		//mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;
		s_Data->RootDomain = nullptr;

		delete s_Data;
	}

}