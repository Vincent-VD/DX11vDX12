#include "pch.h"
#include "BaseGame.h"

BaseGame::BaseGame() noexcept
	: m_window(nullptr)
	, m_outputWidth(800)
	, m_outputHeight(600)
{
	WCHAR assetsPath[512];
}

void BaseGame::OnActivated()
{
}

void BaseGame::OnDeactivated()
{
}

void BaseGame::OnSuspending()
{
}

void BaseGame::OnResuming()
{
}

void BaseGame::OnWindowMoved()
{
}

void BaseGame::OnDisplayChange()
{
}

void BaseGame::OnWindowSizeChanged(int width, int height)
{
}

void BaseGame::GetDefaultSize(int& width, int& height) const noexcept
{
	width = 800;
	height = 600;
}