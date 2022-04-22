#pragma once

#include <Murloc.hpp>

class SandboxLayer : public Murloc::Layer {
public:

	SandboxLayer() {}
	~SandboxLayer() {}

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Murloc::Timestep& ts) override;

	void OnImGuiRender() override;

	void OnEvent(Murloc::Event& e) override;
private:
};