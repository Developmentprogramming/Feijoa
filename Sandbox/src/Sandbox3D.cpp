#include "Sandbox3D.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

Sandbox3D::Sandbox3D()
	: Layer("Sandbox3D"), m_CameraController({ 0.0f, 0.0f, 0.0f }, 1280.0f / 720.0f)
{
}

void Sandbox3D::OnAttach()
{
	FJ_PROFILE_FUNCTION();

	m_CheckerboardTexture = Feijoa::Texture2D::Create("assets/textures/Checkerboard.png");

	m_ActiveScene = Feijoa::CreateRef<Feijoa::Scene>();
	m_Camera = m_ActiveScene->CreateEntity("Perspective Camera");
	m_Camera.AddComponent<Feijoa::PerspectiveCameraComponent>(glm::vec3(0.0f, 0.0f, 0.0f), 1280.0f/720.0f);

	class CameraController : public Feijoa::ScriptableEntity
	{
	public:
		void OnCreate()
		{
		}

		void OnDestroy()
		{
		}

		void OnUpdate(Feijoa::Timestep ts)
		{
			auto& camera = GetComponent<Feijoa::PerspectiveCameraComponent>().Camera;

			if (Feijoa::Input::IsKeyPressed(Feijoa::KeyCode::W))
				camera.UpdateDirection(Feijoa::PerspectiveSceneCamera::Direction::Forward, ts);
			if (Feijoa::Input::IsKeyPressed(Feijoa::KeyCode::S))
				camera.UpdateDirection(Feijoa::PerspectiveSceneCamera::Direction::Backward, ts);
			if (Feijoa::Input::IsKeyPressed(Feijoa::KeyCode::A))
				camera.UpdateDirection(Feijoa::PerspectiveSceneCamera::Direction::Left, ts);
			if (Feijoa::Input::IsKeyPressed(Feijoa::KeyCode::D))
				camera.UpdateDirection(Feijoa::PerspectiveSceneCamera::Direction::Right, ts);
			if (Feijoa::Input::IsKeyPressed(Feijoa::KeyCode::Space))
				camera.UpdateDirection(Feijoa::PerspectiveSceneCamera::Direction::Top, ts);
			if (Feijoa::Input::IsKeyPressed(Feijoa::KeyCode::LeftShift))
				camera.UpdateDirection(Feijoa::PerspectiveSceneCamera::Direction::Bottom, ts);

			auto [x, y] = Feijoa::Input::GetMousePosition();
			float dx = x - m_LastX;
			float dy = m_LastY - y; // Because y is inverted
			m_LastX = x;
			m_LastY = y;
			camera.UpdatePosition(dx, dy);
		}

	private:
		float m_LastX = 0, m_LastY = 0;
	};
	m_Camera.AddComponent<Feijoa::NativeScriptComponent>().Bind<CameraController>();
}

void Sandbox3D::OnDetach()
{
	FJ_PROFILE_FUNCTION();
}

void Sandbox3D::OnUpdate(Feijoa::Timestep ts)
{
	FJ_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	Feijoa::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Feijoa::RenderCommand::Clear();

	m_ActiveScene->OnUpdate(ts);
	
}

void Sandbox3D::OnEvent(Feijoa::Event& e)
{
	m_CameraController.OnEvent(e);
}

void Sandbox3D::OnImGuiRender()
{
	FJ_PROFILE_FUNCTION();

	auto& camera = m_CameraController.GetCamera();
	ImGui::Begin("Camera control");
	ImGui::DragFloat("FOV", &m_FOV, 1.0f, camera.GetMinFOV(), camera.GetMaxFOV());
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	auto stats = Feijoa::Renderer3D::GetStats();
	ImGui::Text("Renderer 3D Stats:");
	ImGui::Text("Draw calls: %d", stats.DrawCalls);
	ImGui::Text("Quad count: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::End();
	camera.SetFOV(m_FOV);
}
