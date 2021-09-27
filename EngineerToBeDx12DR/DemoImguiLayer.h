#pragma once

#include "ImguiLayerBase.h"

class DemoImguiLayer : public ImguiLayerBase
{
public:
	DemoImguiLayer(bool showDemoWindow, bool showAnotherWindow, ImVec4 clearColor);
	
	~DemoImguiLayer() override = default;

	void CreateGUI() override;

private:
	bool m_showDemoWindow;
	bool m_showAnotherWindow;
	ImVec4 m_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};
