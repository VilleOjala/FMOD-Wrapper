MIT License
Copyright (c) 2020 Ville Ojala

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "id_system.h"

struct DialogueUserData
{
	bool is_3d;
	std::string line_key;
	unsigned long int associated_event_id;
};

class WrapperImplementation
{
private:

	WrapperImplementation();
	~WrapperImplementation();

	void runUpdate();

	FMOD::Studio::System* studio_system;
	FMOD::System* core_system;

	// Each event instance will be assigned with a unique id for later access (e.g. to update positional and parameter data)
	std::map<unsigned long int, FMOD::Studio::EventInstance*> m_events;

	std::map<std::string, FMOD::Studio::Bank*> m_banks;
	std::map<unsigned long int, DialogueUserData*> m_alloc_dialogue_user_data;

public:

	friend class FmodWrapper;
};

class FmodWrapper
{
private:

	static void initializeIdSystem();
	static int setNumberOfListeners(int num_listeners);	
	static bool audio_engine_initialized;
	
	// To be used as a default argument for all "play sound" -functions when the caller does not provide any fmod parameters to set.  
	static std::map<std::string, float> empty_map;

public:

	// Passing arguments by value vs. reference should be re-evaluated based on the call system implementation on the game engine side. 

	static void initializeAudioEngine();
	static void callUpdate();
	static void shutDownAudioEngine();
	static int errorCheck(FMOD_RESULT result);

	int loadBank(const std::string& bank, bool load_samples = true);
	int unloadBank(const std::string& bank);
	int loadSampleData(const std::string& bank);
	int unloadSampleData(const std::string& bank);

	unsigned long int play3DEvent(const std::string& event, FMOD_3D_ATTRIBUTES spatial_attributes, std::map<std::string, float> parameters = empty_map);
	unsigned long int play2DEvent(const std::string& event, std::map<std::string, float> parameters = empty_map);
	int stopEvent(int event_id, bool allow_fades = true);
	
	int set3DAttributes(int event_id, FMOD_3D_ATTRIBUTES spatial_attributes);
	static int setListenerAttributes(int listener_index, FMOD_3D_ATTRIBUTES spatial_attributes); 
	
	int setParameterByName(int event_id, std::string& parameter, float value);
	int setGlobalParameterByName(std::string& parameter, float value);

	// These are commonly needed when implementing main and pause menu systems.
	int setBusPauseStatus(const std::string& bus, bool is_paused);
	int stopAllBusEvents(const std::string& bus, bool allow_fades);
	

	// Programmer sound / audio table system for voiceovers -->

	// Create an enum value for each programmer sound instrument you want to use in FMOD Studio for dialogue mixer routing and other speaker/situation specific processing.
	enum DialogueMasterEvents
	{
		//For example...

		PC,
		NPC,
		NPC_radio

		//...etc.....	
	};
	
	static FMOD_RESULT F_CALLBACK dialogueEventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void* parameter);

	unsigned long int playDialogue3D(const std::string key, DialogueMasterEvents master_event, FMOD_3D_ATTRIBUTES spatial_attributes, std::map<std::string, float> parameters = empty_map);
	unsigned long int playDialogue2D(const std::string key, DialogueMasterEvents master_event, std::map<std::string, float> parameters = empty_map);

};
