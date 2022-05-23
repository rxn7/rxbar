#pragma once
#include <string>
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point TimePoint;

struct ModuleInfo {
	std::string icon;
	std::string command;
	uint32_t intervalMs;
};

struct Module {
	const ModuleInfo *info;
	std::string output;
	TimePoint lastUpdate;
};
