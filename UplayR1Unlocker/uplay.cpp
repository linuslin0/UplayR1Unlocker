#include "pch.h"
#include "uplay.h"
#include "Logger.h"
#include "Config.h"

HMODULE originalDLL = nullptr;

void init(HMODULE hModule)
{
	Config::init(hModule);
	Logger::init(hModule);

	logger->info("Uplay R1 Unlocker v{}", VERSION);

	auto originalPath = getDllDir(hModule) / ORIG_DLL;
	originalDLL = LoadLibrary(originalPath.c_str());
	if(originalDLL)
	{
		logger->info(L"Successfully loaded original DLL: {}", ORIG_DLL);
	}
	else
	{
		auto message = fmt::format(L"Failed to load original DLL: {}. Error code: {}", originalPath.c_str(), GetLastError());
		logger->error(message);
		MessageBox(NULL, message.c_str(), L"Error during initialization", MB_ICONERROR | MB_OK);
		exit(1);
	}
}


void shutdown()
{
	logger->info("Shutting down");
	FreeLibrary(originalDLL);
}

// Used as: cmp eax, 1
// Therefore, we must return int rather than a bool
EXPORT int UPLAY_USER_IsOwned(int aUplayId)
{
	GET_PROXY_FUNC(UPLAY_USER_IsOwned);
	auto result = proxyFunc(aUplayId);

	auto isOwned = !vectorContains(config->blacklist, aUplayId);

	logger->info(
		"UPLAY_USER_IsOwned -> aUplayId: {},\tisOwned: {}\t(legitimately owned: {})",
		aUplayId, isOwned, (bool) result
	);

	return isOwned; // implicit type conversion
}

EXPORT PCSTR UPLAY_INSTALLER_GetLanguageUtf8()
{
	if(config->lang == "default")
	{
		GET_PROXY_FUNC(UPLAY_INSTALLER_GetLanguageUtf8);
		auto result = proxyFunc();
		logger->info("UPLAY_INSTALLER_GetLanguageUtf8 -> original lang: {}", result);
		return result;
	}
	else
	{
		logger->info("UPLAY_INSTALLER_GetLanguageUtf8 -> modified lang: {}", config->lang);
		return config->lang.c_str();
	}
}

#ifdef LOADER
// only need this for loader (per game)
EXPORT int UPLAY_Startup(int aUplayId, int aBuildId, const char * lang_utf8)
{
	GET_PROXY_FUNC(UPLAY_Startup);

	auto use_uplay_id = aUplayId;
	if (config->uplay_id != 0 && config->uplay_id != aUplayId) {
		use_uplay_id = config->uplay_id;
	}

	auto result = proxyFunc(use_uplay_id, aBuildId, lang_utf8);
	if (use_uplay_id == aUplayId) {
		logger->info("UPLAY_Startup -> UplayID: {}, result: {}", use_uplay_id, result);
	} else {
		logger->info("UPLAY_Startup -> modified UplayID: {}, result: {}", use_uplay_id, result);
	}
	
	return result;
}
#endif
