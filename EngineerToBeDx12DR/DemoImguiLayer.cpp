﻿#include "pch.h"
#include "DemoImguiLayer.h"

#include "imgui.h"


DemoImguiLayer::DemoImguiLayer(bool showDemoWindow, bool showAnotherWindow, ImVec4 clearColor)
	: ImguiLayerBase(),
	  m_showDemoWindow(showDemoWindow), m_showAnotherWindow(showAnotherWindow), m_clearColor(clearColor) {}

void DemoImguiLayer::CreateGUI()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (m_showDemoWindow)
		ImGui::ShowDemoWindow(&m_showDemoWindow);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &m_showDemoWindow); // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &m_showAnotherWindow);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&m_clearColor); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))
			// Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text(
			"Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate
		);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (m_showAnotherWindow)
	{
		ImGui::Begin("Another Window", &m_showAnotherWindow);
		// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			m_showAnotherWindow = false;
		ImGui::End();
	}
}
