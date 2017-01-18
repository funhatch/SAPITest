#include "stdafx.h"
#include "SAPIEngine.h"

#pragma warning( push )
#pragma warning( disable : 4996 ) // Disable warning about deprecated 'GetVersionExW'
#include <sphelper.h>
#pragma warning( pop )

#include <string>

SAPIEngine::SAPIEngine() :
	_nextRuleId(0)
{
}

SAPIEngine::~SAPIEngine()
{
	shutdown();
}

SAPIEngine& SAPIEngine::getInstance()
{
	static SAPIEngine instance;
	return instance;
}

bool SAPIEngine::initialize()
{
	bool initialized = false;

	// Initialize COM
	if (!FAILED(::CoInitialize(nullptr)))
	{
		// Use the shared speech recognition context
		HRESULT hr = _recoContext.CoCreateInstance(CLSID_SpSharedRecoContext);
		if (SUCCEEDED(hr))
		{
			// Tell the context that we are only interested in speech recognition events
			_recoContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

			// Create a speech recognition grammar
			hr = _recoContext->CreateGrammar(0, &_grammar.p);
			if (SUCCEEDED(hr))
			{
				// Use a blank grammar for now
				_grammar->ResetGrammar(SpGetUserDefaultUILanguage());

				initialized = true;
			}
		}
	}

	return initialized;
}

void SAPIEngine::shutdown()
{
	// Release the speech recognition interfaces
	_grammar.Release();
	_recoContext.Release();

	// Shut down COM
	::CoUninitialize();
}

void SAPIEngine::update()
{
	if (!_recoContext.p)
	{
		return;
	}

	// Get all events from the Recognition Context
	CSpEvent event;
	while (event.GetFrom(_recoContext) == S_OK)
	{
		// Process valid speech recognition events
		if (event.eEventId == SPEI_RECOGNITION && event.RecoResult())
		{
			ISpRecoResult* recognitionResult = event.RecoResult();

			// Get the text string from the phrase
			WCHAR* phraseText = nullptr;
			HRESULT hr = recognitionResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, true, &phraseText, nullptr);
			if (SUCCEEDED(hr))
			{
				// Call back to client
				onRecognized(std::wstring(phraseText));
				// Free the phrase memory
				CoTaskMemFree(phraseText);
			}
		}
	}
}

int SAPIEngine::addCommand(const std::wstring& command)
{
	int newCommandId = -1;
	if (_grammar.p)
	{
		// Make a unique rule ID
		++_nextRuleId;

		// Add a new dynamic rule to the grammar
		SPSTATEHANDLE hState = 0;
		HRESULT hr = _grammar->GetRule(nullptr, _nextRuleId, SPRAF_TopLevel | SPRAF_Dynamic, TRUE, &hState);
		if (SUCCEEDED(hr))
		{
			// Add a word or sequence of words to the grammar
			hr = _grammar->AddWordTransition(hState, 0, command.c_str(), L" ", SPWT_LEXICAL, 1.0f, nullptr);
			if (SUCCEEDED(hr))
			{
				hr = _grammar->Commit(0);
				if (SUCCEEDED(hr))
				{
					// Make the rule active
					_grammar->SetRuleIdState(_nextRuleId, SPRS_ACTIVE);

					// Record it in our list
					_commands.push_back(std::make_pair(_nextRuleId, command));
					newCommandId = _nextRuleId;
				}
			}
		}
	}
	return newCommandId;
}

void SAPIEngine::setCallback(SAPICallback callbackFunc)
{
	_callback = callbackFunc;
}

void SAPIEngine::onRecognized(const std::wstring& text)
{
	if (_callback)
	{
		// Find the recognized command in our list
		int foundRuleID = -1;
		for (auto& p : _commands)
		{
			if (p.second == text)
			{
				foundRuleID = p.first;
				break;
			}
		}
		if (foundRuleID != -1)
		{
			_callback(foundRuleID, text);
		}
	}
}
