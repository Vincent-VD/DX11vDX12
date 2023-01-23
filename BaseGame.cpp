#include "pch.h"
#include "BaseGame.h"

BaseGame::BaseGame() noexcept
	: m_Window(nullptr)
	, m_OutputWidth(800)
	, m_OutputHeight(600)
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
	m_OutputWidth = width;
	m_OutputHeight = height;
}

void BaseGame::GetDefaultSize(int& width, int& height) const noexcept
{
	width = 800;
	height = 600;
}

void BaseGame::GetCurrentWindowSize(int& width, int& height) const noexcept
{
	width = m_OutputWidth;
	height = m_OutputHeight;
}
