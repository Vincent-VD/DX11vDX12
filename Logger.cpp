#include "pch.h"
#include "Logger.h"
#include <iostream>
#include <sstream>

#include "GameDX11.h"
#include "GameDX12.h"

Logger* Logger::m_Instance = nullptr;

Logger* Logger::GetInstance()
{
	if (m_Instance == nullptr)
	{
		m_Instance = new Logger();
	}
	return m_Instance;
}

void Logger::Release()
{
	delete m_Instance;
}

Logger::Logger()
{
	m_FileStream.open(m_FileName.c_str());
	m_FileStream << "Total time; FPS; Instances; Triangle Count\n";
}

Logger::~Logger()
{
	m_FileStream.close();
}


bool Logger::Update(DX::StepTimer timer)
{
	m_CurrTime += static_cast<float>(timer.GetElapsedSeconds());
	if(m_CurrTime >= m_LogInterval)
	{
		m_CurrTime = 0.f;
		return true; //log this frame
	}
	return false;
}

void Logger::Log(DX::StepTimer timer, const GameDX11* pDX11, const GameDX12* pDX12)
{
	std::cout << "logging\n";
	const uint32_t fps{ timer.GetFramesPerSecond() };
	const auto totalTime{ static_cast<uint32_t>(timer.GetTotalSeconds())};
	uint32_t Instances{};

	if (pDX11)
	{
		Instances = pDX11->GetCurrentInstanceCount();
	} else
	{
		Instances = pDX12->GetCurrentInstanceCount();
	}

	const uint32_t CurrTriangleCount{ Instances * 12 };


	std::stringstream stream{};
	stream << std::to_string(totalTime) << "; " << std::to_string(fps) << "; " << std::to_string(Instances) << "; " << std::to_string(CurrTriangleCount) << "\n";
	m_FileStream << stream.rdbuf();
}
