// SAPITest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SAPIEngine.h"

#include <chrono>
#include <iostream>
#include <thread>

bool g_Quit = false;

void sapiCallback(int matchedRuleID, const std::wstring& text)
{
	if (text == L"quit")
	{
		g_Quit = true;
	}

	std::cout << matchedRuleID << ": ";
	// This is a hacky method to output the wstring to regular std::cout
	for (auto c : text)
	{
		std::cout << (char)c;
	}
	std::cout << std::endl;
}

int main()
{
	SAPIEngine& sapi = SAPIEngine::getInstance();
	sapi.initialize();

	sapi.addCommand(L"quit");
	sapi.addCommand(L"computer");
	sapi.addCommand(L"hello computer");
	sapi.addCommand(L"open the pod bay doors please hal");
	sapi.addCommand(L"primary weapon");
	sapi.addCommand(L"secondary weapon");
	sapi.addCommand(L"tertiary weapon");

	sapi.setCallback(sapiCallback);
	while (!g_Quit)
	{
		sapi.update();
	}
	return 0;
}

