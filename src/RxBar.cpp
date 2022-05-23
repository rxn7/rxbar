#include <sstream>
#include <iostream>
#include <chrono>
#include <signal.h>
#include <X11/Xlib.h>

#include "Modules.h"

#define MAX_OUTPUT_LENGTH 32
#define ERROR(msg) std::cout << "rxstatus error: " << (msg) << "\n"

static constexpr uint32_t MODULE_COUNT = sizeof(moduleInfos) / sizeof(ModuleInfo);

static Display *display;
static Window root;
static int screen;
static bool continueStatusLoop = true;
static std::stringstream statusStream;
static std::string lastStatusText;
static Module modules[MODULE_COUNT];

void StatusLoop();
void WriteStatus();
bool UpdateStatusStream(); // Returns true if status text has changed  
void UpdateOutputs();
void UpdateModule(Module *mod);
void SetupModules();
bool SetupX();
void CloseX();
void SigHandler(int signal);

int main(int argc, const char **argv) {
	if(!SetupX())
		return -1;

	SetupModules();

	StatusLoop();

	CloseX();
}

void StatusLoop() {
	while(continueStatusLoop) {
		UpdateOutputs();
		WriteStatus();
		usleep(1000);
	}
}

void WriteStatus() {
	if(!UpdateStatusStream()) // Don't write the status if it hasn't changed
		return;

	XStoreName(display, root, statusStream.str().c_str());
	XFlush(display);
}

bool UpdateStatusStream() {
	lastStatusText = statusStream.str();
	
	statusStream.str(std::string());
	statusStream.clear();

	for(uint32_t i=0; i<MODULE_COUNT; ++i) {
		statusStream << "[ " << modules[i].info->icon << " " << modules[i].output << " ]";

		if(i != MODULE_COUNT-1)
			statusStream << "   ";
	}

	return statusStream.str() == lastStatusText;
}

void UpdateOutputs() {
	auto now = std::chrono::high_resolution_clock::now();

	Module *mod;
	for(uint32_t i=0; i<MODULE_COUNT; ++i) {
		mod = &modules[i];

		uint32_t timeDeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - mod->lastUpdate).count();
		if(timeDeltaMs >= mod->info->intervalMs)
			UpdateModule(mod);
	}
}

void UpdateModule(Module *mod) {
	mod->lastUpdate = std::chrono::high_resolution_clock::now();

	FILE *cmd = popen(mod->info->command.c_str(), "r");
	if(!cmd) {
		mod->output = "error running command";
		return;
	}

	char output[MAX_OUTPUT_LENGTH];
	fgets(output, MAX_OUTPUT_LENGTH, cmd);
	pclose(cmd);

	mod->output = std::string(output);

	if(mod->output.at(mod->output.length()-1) == '\n')
		mod->output.pop_back();
}

void SetupModules() {
	Module *mod;
	for(uint32_t i=0; i<MODULE_COUNT; ++i) {
		mod = &modules[i];
		mod->info = &moduleInfos[i];
		UpdateModule(mod);
	}
}

bool SetupX() {
	display = XOpenDisplay(NULL);
	if(!display) {
		ERROR("Failed to open display");
		return false;
	}

	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	signal(SIGTERM, SigHandler);
	signal(SIGINT, SigHandler);

	return true;
}

void CloseX() {
	XCloseDisplay(display);
}

void SigHandler(int signal) {
	continueStatusLoop = false;
}
