#pragma once
#include "util.h"

class Config
{
protected:
	Config(HMODULE hModule);
public:
	string log_level = "off";
	string lang = "default";
	vector<int> blacklist;
	int uplay_id = 0;

	static void init(HMODULE hModule);
};

extern shared_ptr<Config> config;
