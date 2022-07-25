#include "EditorLayer.h"

#include <Pangolin.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace PG {
    
	static Timestep g_Timestep{ 0 };
    
	void EditorLayer::OnAttach()
	{
		auto& specs = Application::Get()->GetSpecification();
		m_EditorCamera = EditorCamera(30.0f, (float)specs.Width/ (float)specs.Height, 0.1f, 10000.0f);
		m_ViewportSize = { specs.Width, (float)specs.Height };
		
		m_EditorScene = CreateRef<Scene>();
		
		m_SceneHierarchyPanel.SetCurrentScene(m_EditorScene);

		// Set active scene 
		m_ActiveScene = m_EditorScene;
	}
    
	void EditorLayer::OnDetach()
	{
	}
	
	void EditorLayer::OnUpdate(Timestep& ts)
	{
		g_Timestep = ts;
        
		m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		m_EditorCamera.OnUpdate(ts);
		
		switch (m_SceneMode)
		{
		case SceneMode::Edit:
			m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
			break;
		case SceneMode::Play:
			break;
		default:
			break;
		}
	}
    
	void EditorLayer::OnImGuiRender()
	{
		ImGui_BeginDockspace();
        
		// Scene Statistics & timestep
		{
			auto& sceneStats = m_ActiveScene->GetStatistics();
			ImGui::Begin("Active Scene Debug Info");
			ImGui::Text("Entities: %d", sceneStats.Entities);
			ImGui::Text("Rendered Entities: %d", sceneStats.RenderedEntities);
			ImGui::Text("Last Frame: %f ms", g_Timestep.GetMs());
			ImGui::End();
		}
		
		// Scene hierarchy panel
		m_SceneHierarchyPanel.OnImGuiRender();
		
		{
			ImGui::Begin("File Manager");
			ImGui::End();
		}
		
		// Viewport
		{
			ImGuiWindowFlags windowFlags = 0;
			windowFlags |= ImGuiWindowFlags_NoTitleBar;
			windowFlags |= ImGuiWindowFlags_NoMove;
			
			ImGuiWindowClass window_class;
			window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
			ImGui::SetNextWindowClass(&window_class);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });// removes border
			ImGui::Begin("Viewport", 0, windowFlags);
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x , viewportPanelSize.y };
			{
				// Descriptor set pointer
				SceneID renderedTexture = m_ActiveScene->GetRenderedScene();
				ImGui::Image(renderedTexture, { viewportPanelSize.x,viewportPanelSize.y });
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}
		ImGui::End();
	}
    
	void EditorLayer::OnEvent(Event& e)
	{
		m_SceneHierarchyPanel.OnEvent(e);
		m_EditorCamera.OnEvent(e);
	}

	void EditorLayer::ImGui_BeginDockspace()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
        
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;
		
		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleVar();
        
		ImGui::PopStyleVar(2);
        
		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
			
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWinSizeX;
	}
}