#ifndef __SAPIENGINE_H__
#define __SAPIENGINE_H__

#pragma comment(lib, "sapi.lib")

#include <atlcom.h>
#include <sapi.h>
#include <string>
#include <vector>

class SAPIEngine
{
public:

	// Singleton access
	static SAPIEngine& getInstance();

	// Initialize the SAPI engine for the user's default language
	bool initialize();

	// Update the SAPI engine
	void update();

	// Shutdown and cleanup
	void shutdown();

	// Add a new command to the speech recognition grammar
	// Returns the ID assigned to the new command, or -1 on failure
	int addCommand(const std::wstring& command);

	// Set a callback to inform the client when a word or phrase
	// has been recognized by SAPI. The first parameter is the ID
	// of the recognized command, as returned when the command was added,
	// and the second is the recognized phrase itself.
	using SAPICallback = void(*)(int, const std::wstring&);

	void setCallback(SAPICallback callbackFunc);

private:

	SAPIEngine();
	SAPIEngine(const SAPIEngine&); // Not implemented
	SAPIEngine& operator=(const SAPIEngine&); // Not implmented
	~SAPIEngine();
	void onRecognized(const std::wstring& text);

	CComPtr<ISpRecoContext> _recoContext;
	CComPtr<ISpRecoGrammar> _grammar;
	SAPICallback _callback;
	std::vector<std::pair<int, std::wstring>> _commands;
	int _nextRuleId;
};

#endif // __SAPIENGINE_H__
