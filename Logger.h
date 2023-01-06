#pragma once
#include "pch.h"
#include <fstream>

class GameDX11;
class GameDX12;

class Logger
{
public:

	static Logger* GetInstance();
	void Release();

	~Logger();
	Logger(const Logger& other) = delete;
	Logger(Logger&& other) noexcept = delete;
	Logger& operator=(const Logger& other) = delete;
	Logger& operator=(Logger&& other) noexcept = delete;

	bool Update(DX::StepTimer timer);
	void Log(DX::StepTimer timer, const GameDX11* pDX11, const GameDX12* pDX12);


private:
	Logger();

	static Logger* m_Instance;

	float m_CurrTime{};
	float m_LogInterval{ 1.f }; //log every 1 sec
	std::string m_FileName{ "perf.csv" };
	std::ofstream m_FileStream{};

};

