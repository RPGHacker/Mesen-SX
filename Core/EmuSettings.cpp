#include "stdafx.h"
#include "EmuSettings.h"
#include "KeyManager.h"
#include "MessageManager.h"
#include "../Utilities/FolderUtilities.h"

void EmuSettings::ProcessString(string & str, const char ** strPointer)
{
	//Make a copy of the string and keep it (the original pointer will not be valid after the call is over)
	str = *strPointer;
	*strPointer = str.c_str();
}

void EmuSettings::SetVideoConfig(VideoConfig config)
{
	_video = config;
}

VideoConfig EmuSettings::GetVideoConfig()
{
	return _video;
}

void EmuSettings::SetAudioConfig(AudioConfig config)
{
	ProcessString(_audioDevice, &config.AudioDevice);

	_audio = config;
}

AudioConfig EmuSettings::GetAudioConfig()
{
	return _audio;
}

void EmuSettings::SetEmulationConfig(EmulationConfig config)
{
	_emulation = config;
}

EmulationConfig EmuSettings::GetEmulationConfig()
{
	return _emulation;
}

void EmuSettings::SetPreferences(PreferencesConfig config)
{
	ProcessString(_saveFolder, &config.SaveFolderOverride);
	ProcessString(_saveStateFolder, &config.SaveStateFolderOverride);
	ProcessString(_screenshotFolder, &config.ScreenshotFolderOverride);

	MessageManager::SetOsdState(!config.DisableOsd);

	FolderUtilities::SetFolderOverrides(
		_saveFolder,
		_saveStateFolder,
		_screenshotFolder
	);

	_preferences = config;
}

PreferencesConfig EmuSettings::GetPreferences()
{
	return _preferences;
}

void EmuSettings::ClearShortcutKeys()
{
	_emulatorKeys[0].clear();
	_emulatorKeys[1].clear();
	_emulatorKeys[2].clear();
	_shortcutSupersets[0].clear();
	_shortcutSupersets[1].clear();
	_shortcutSupersets[2].clear();

	//Add Alt-F4 as a fake shortcut to prevent Alt-F4 from triggering Alt or F4 key bindings. (e.g load save state 4)
	KeyCombination keyComb;
	keyComb.Key1 = KeyManager::GetKeyCode("Alt");
	keyComb.Key2 = KeyManager::GetKeyCode("F4");
	SetShortcutKey(EmulatorShortcut::Exit, keyComb, 2);
}

void EmuSettings::SetShortcutKey(EmulatorShortcut shortcut, KeyCombination keyCombination, int keySetIndex)
{
	_emulatorKeys[keySetIndex][(uint32_t)shortcut] = keyCombination;

	for(int i = 0; i < 3; i++) {
		for(std::pair<const uint32_t, KeyCombination> &kvp : _emulatorKeys[i]) {
			if(keyCombination.IsSubsetOf(kvp.second)) {
				_shortcutSupersets[keySetIndex][(uint32_t)shortcut].push_back(kvp.second);
			} else if(kvp.second.IsSubsetOf(keyCombination)) {
				_shortcutSupersets[i][kvp.first].push_back(keyCombination);
			}
		}
	}
}

void EmuSettings::SetShortcutKeys(vector<ShortcutKeyInfo> shortcuts)
{
	ClearShortcutKeys();

	for(ShortcutKeyInfo &shortcut : shortcuts) {
		if(_emulatorKeys[0][(uint32_t)shortcut.Shortcut].GetKeys().empty()) {
			SetShortcutKey(shortcut.Shortcut, shortcut.KeyCombination, 0);
		} else {
			SetShortcutKey(shortcut.Shortcut, shortcut.KeyCombination, 1);
		}
	}
}

KeyCombination EmuSettings::GetShortcutKey(EmulatorShortcut shortcut, int keySetIndex)
{
	auto result = _emulatorKeys[keySetIndex].find((int)shortcut);
	if(result != _emulatorKeys[keySetIndex].end()) {
		return result->second;
	}
	return {};
}

vector<KeyCombination> EmuSettings::GetShortcutSupersets(EmulatorShortcut shortcut, int keySetIndex)
{
	return _shortcutSupersets[keySetIndex][(uint32_t)shortcut];
}

uint32_t EmuSettings::GetEmulationSpeed()
{
	return _emulation.EmulationSpeed;
}

double EmuSettings::GetAspectRatio()
{
	switch(_video.AspectRatio) {
		case VideoAspectRatio::NoStretching: return 0.0;

		case VideoAspectRatio::Auto:
		{
			bool isPal = false;
			return isPal ? (9440000.0 / 6384411.0) : (128.0 / 105.0);
		}

		case VideoAspectRatio::NTSC: return 128.0 / 105.0;
		case VideoAspectRatio::PAL: return 9440000.0 / 6384411.0;
		case VideoAspectRatio::Standard: return 4.0 / 3.0;
		case VideoAspectRatio::Widescreen: return 16.0 / 9.0;
		case VideoAspectRatio::Custom: return _video.CustomAspectRatio;
	}
	return 0.0;
}