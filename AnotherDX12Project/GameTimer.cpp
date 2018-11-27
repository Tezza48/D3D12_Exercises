#include "GameTimer.h"
#include <Windows.h>

GameTimer::GameTimer() : secondsPerCount(0.0), deltaTime(-1.0), baseTime(0), pausedTime(0), stopTime(0), prevTime(0), currTime(0), isStopped(false)
{
	__int64 countsPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSecond);
	secondsPerCount = 1.0 / static_cast<double>(countsPerSecond);
}

float GameTimer::TotalTime() const
{
	if (isStopped)
	{
		return static_cast<float>((stopTime - pausedTime) - baseTime) * secondsPerCount;
	}
	else
	{
		return static_cast<float>((currTime - pausedTime) - baseTime) * secondsPerCount;
	}
}

float GameTimer::DeltaTime() const
{
	return static_cast<float>(deltaTime);
}

void GameTimer::Reset()
{
	__int64 newCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&newCurrTime);

	baseTime = newCurrTime;
	prevTime = newCurrTime;
	stopTime = 0;
	isStopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime);
	if (isStopped)
	{
		pausedTime += (startTime - stopTime);
		prevTime = startTime;

		stopTime = 0;
		isStopped = false;
	}
}

void GameTimer::Stop()
{
	if (!isStopped)
	{
		__int64 newCurrTime;
		QueryPerformanceCounter((LARGE_INTEGER *)&newCurrTime);
		stopTime = newCurrTime;
		isStopped = true;
	}
}

void GameTimer::Tick()
{
	if (isStopped)
	{
		deltaTime = 0.0;
		return;
	}
	__int64 newCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&newCurrTime);
	currTime = newCurrTime;
	
	deltaTime = (currTime - prevTime) * secondsPerCount;

	prevTime = currTime;

	if (deltaTime < 0.0)
	{
		deltaTime = 0;
	}
}
