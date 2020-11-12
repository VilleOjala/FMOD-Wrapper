
#include "fmod_wrapper.h"

WrapperImplementation* audio_engine = nullptr;
IdSystem* id_system = nullptr;

bool FmodWrapper::audio_engine_initialized = false;
std::map<std::string, float> FmodWrapper::empty_map;

WrapperImplementation::WrapperImplementation()
{
	studio_system = nullptr;
	FmodWrapper::errorCheck(FMOD::Studio::System::create(&studio_system));
	core_system = nullptr;
	FmodWrapper::errorCheck(studio_system->getCoreSystem(&core_system));
	FmodWrapper::errorCheck(studio_system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL));
	core_system->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, 0);
}

WrapperImplementation::~WrapperImplementation()
{
	FmodWrapper::errorCheck(studio_system->unloadAll());
	FmodWrapper::errorCheck(studio_system->release());

	if (!m_alloc_dialogue_user_data.empty())
	{
		for (auto it = m_alloc_dialogue_user_data.begin(); it != m_alloc_dialogue_user_data.end(); ++it)
		{
			if (it->second != nullptr)
			{
				DialogueUserData* data = it->second;
				delete data;
				it->second = nullptr;
			}
		}
		m_alloc_dialogue_user_data.clear();
	}
}

void WrapperImplementation::runUpdate()
{
	if (!m_events.empty())
	{
		for (auto it = m_events.begin(); it != m_events.end();)
		{
			bool event_valid = it->second->isValid();

			if (event_valid == false) 
			{
				std::cout << "Erased ID: " << it->first << std::endl; // Temp debug print.
				m_events.erase(it++);
				continue;
			}

			FMOD_STUDIO_PLAYBACK_STATE pb_state;
			it->second->getPlaybackState(&pb_state);

			if (pb_state == FMOD_STUDIO_PLAYBACK_STOPPED)
			{
				FmodWrapper::errorCheck(it->second->release());
				std::cout << "Erased ID: " << it->first << std::endl; // Temp debug print.
				m_events.erase(it++);
				continue;
			}

			++it;
		}
	}
	studio_system->update();
}


void FmodWrapper::initializeAudioEngine()
{
	if (audio_engine_initialized)
	{
		// Warn here about trying to initialize multiple audio engines.
		return;
	}

	audio_engine = new WrapperImplementation;
	bool engine_is_valid = audio_engine->studio_system->isValid();
	if (!engine_is_valid) 
	{
		// Add game engine console error message here.
	}
	else
	{	
		initializeIdSystem();

		int e;
		
		// Load master bank and the master string bank. Add project specific locations here.
		std::string master_bank_location = "D:/FmodTestProject/Build/Desktop/Master.bank";
		std::string master_bank_strings_location = "D:/FmodTestProject/Build/Desktop/Master.strings.bank";
		
		FMOD::Studio::Bank* b_master = nullptr;
		e = errorCheck(audio_engine->studio_system->loadBankFile(master_bank_location.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &b_master));
		if (e == 1) { return; }

		FMOD::Studio::Bank* b_master_strings = nullptr;
		e = errorCheck(audio_engine->studio_system->loadBankFile(master_bank_strings_location.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &b_master_strings));
		if (e == 1) { return; }

		audio_engine->m_banks[master_bank_location] = b_master;
		audio_engine->m_banks[master_bank_strings_location] = b_master_strings;

		// Hard coded with one listener, change as necessary;
		e = setNumberOfListeners(1);
		// If setting up listeners failed, abort initialization. Add error message to the game engine console.
		if (e == 0) { return; }

		// Setting up some default listener 3D attributes for debugging purposes in the absence of an available game engine. Remove when doing a real integration!
		FMOD_3D_ATTRIBUTES default_attributes;
		default_attributes.position = { 0.0f, 0.0f, 0.0f };
		default_attributes.velocity = { 0.0f, 0.0f, 0.0f };
		default_attributes.forward = { 0.0f, 0.0f, 1.0f };
		default_attributes.up = { 0.0f, 1.0f, 0.0f };

		setListenerAttributes(1, default_attributes);

		audio_engine_initialized = true;
		std::cout << "Audio engine initialized!\n" << std::endl; // Temp debug print.
		audio_engine->runUpdate();
	}
}

void FmodWrapper::initializeIdSystem()
{
	id_system = new IdSystem;
}

int FmodWrapper::setNumberOfListeners(int num_listeners)
{
	int e;
	e = errorCheck(audio_engine->studio_system->setNumListeners(num_listeners));
	if (e == 1) { return 0; }
	return 1;
}

void FmodWrapper::callUpdate()
{
	if (!audio_engine_initialized) { return; }
	audio_engine->runUpdate();
}

void FmodWrapper::shutDownAudioEngine()
{
	if (!audio_engine_initialized) { return; }
	delete audio_engine;
	delete id_system;
	audio_engine = nullptr;
	id_system = nullptr;
	audio_engine_initialized = false;
	std::cout << "Audio engine was shut down!" << std::endl;
}

int FmodWrapper::errorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		// Remember to change the output to the console of the deployed game engine.
		std::cout << "FMOD ERROR " << result << std::endl;
		return 1;
	}
	else
	{
		return 0;
	}
}

int FmodWrapper::loadBank(const std::string& bank, bool load_samples)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_banks.find(bank);

	if (find_key != audio_engine->m_banks.end())
	{
		// Bank is already loaded.
		// If the sample data for this metadata is not yet loaded, use the loadSampleData function.

		return 0;
	}

	FMOD::Studio::Bank* b = nullptr;
	int e;
	e = errorCheck(audio_engine->studio_system->loadBankFile(bank.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &b));
	if (e == 1) { return 0; }

	if (load_samples)
	{
		e = errorCheck(b->loadSampleData());
		if (e == 1)
		{
			b->unload();
			return 0;
		}
		else
		{
			audio_engine->m_banks[bank] = b;
			return 1;
		}
	}
	else
	{
		audio_engine->m_banks[bank] = b;
		return 1;
	}	
}

int FmodWrapper::unloadBank(const std::string& bank)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_banks.find(bank);

	if (find_key == audio_engine->m_banks.end())
	{
		// Bank requested for unloading is not loaded.
		return 0;
	}
	else
	{
		int e;
		e = errorCheck(find_key->second->unload());
		if (e == 1) { return 0; }
		
		audio_engine->m_banks.erase(find_key);
		return 1;
	}
}

int FmodWrapper::loadSampleData(const std::string& bank)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_banks.find(bank);

	if (find_key == audio_engine->m_banks.end())
	{
		// Bank metadata has not been loaded.
		return 0;
	}
	else
	{
		int e;

		FMOD_STUDIO_LOADING_STATE loading_state;
		e = errorCheck(find_key->second->getSampleLoadingState(&loading_state));
		if (e == 1) { return 0; }
		
		switch (loading_state)
		{
			case FMOD_STUDIO_LOADING_STATE_UNLOADING:
				 // Debug message the reason of cancelling the sample data load to the game engine console?
				 return 0;
			case FMOD_STUDIO_LOADING_STATE_UNLOADED:
				 e = errorCheck(find_key->second->loadSampleData());
				 if (e == 1) { return 0; }
				 else { return 1; }		
			case FMOD_STUDIO_LOADING_STATE_LOADING:
				 // Debug message the reason of cancelling the sample data load to the game engine console?
				 return 0;
			case FMOD_STUDIO_LOADING_STATE_LOADED:
			     // Debug message the reason of cancelling the sample data load to the game engine console?
				 return 0;
			case FMOD_STUDIO_LOADING_STATE_ERROR:
				 e = errorCheck(find_key->second->loadSampleData());
				 if (e == 1) { return 0; }
				 else { return 1; }
			default:
				 return 0;
		}
	}
}

int FmodWrapper::unloadSampleData(const std::string& bank)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_banks.find(bank);

	if (find_key == audio_engine->m_banks.end())
	{
		// Bank metadata has not been loaded.
		return 0;
	}
	else
	{
		int e;

		FMOD_STUDIO_LOADING_STATE loading_state;
		e = errorCheck(find_key->second->getSampleLoadingState(&loading_state));
		if (e == 1) { return 0; }

		switch (loading_state)
		{
		case FMOD_STUDIO_LOADING_STATE_UNLOADING:
			// Debug message the reason of cancelling the sample data load to the game engine console?
			return 0;
		case FMOD_STUDIO_LOADING_STATE_UNLOADED:
			// Debug message the reason of cancelling the sample data load to the game engine console?
			return 0;
		case FMOD_STUDIO_LOADING_STATE_LOADING:
			// Debug message the reason of cancelling the sample data load to the game engine console?
			return 0;
		case FMOD_STUDIO_LOADING_STATE_LOADED:
			e = errorCheck(find_key->second->unloadSampleData());
			if (e == 1) { return 0; }
			else { return 1; }
		case FMOD_STUDIO_LOADING_STATE_ERROR:
			e = errorCheck(find_key->second->unloadSampleData());
			if (e == 1) { return 0; }
			else { return 1; }
		default:
			return 0;
		}
	}
}


// "play3dEvent", "play2dEvent", "playDialogue3D" & "playDialogue2D"  -functions return a unique event instance ID to the caller. 
// If necessary, caller can then later use this ID to update FMOD_3D_ATTRIBUTES and local parameters possibly assigned to the particular event instance.
// Initial parameter values can be optionally provided in the function arguments.
// For mixer snapshots, the same play and stop functions can be used for activation and inactivation.

unsigned long int FmodWrapper::play3DEvent(const std::string& event, FMOD_3D_ATTRIBUTES spatial_attributes, std::map<std::string, float> parameters)
{
	if (!audio_engine_initialized) { return 0; }

	FMOD::Studio::EventDescription* event_description = nullptr;
	int e = errorCheck(audio_engine->studio_system->getEvent(event.c_str(), &event_description));
	if (e == 1) { return 0; }

	FMOD::Studio::EventInstance* event_instance = nullptr;
	e = errorCheck(event_description->createInstance(&event_instance));
	if (e == 1) { return 0; }

	bool is_3d = false;
	event_description->is3D(&is_3d);

	if (is_3d)
	{
		e = errorCheck(event_instance->set3DAttributes(&spatial_attributes));
		if (e == 1) { return 0; }
	}
	else
	{
		return 0;
	}

	if (!parameters.empty())
	{
		for(auto it = parameters.begin();  it != parameters.end(); it++)
		{
			std::string parameter = it->first;
			float value = it->second;
			errorCheck(event_instance->setParameterByName(parameter.c_str(), value, false));
		}
	}

	if (id_system != nullptr)
	{
		unsigned long int id = id_system->getUniqueId();
		audio_engine->m_events.insert(std::pair<unsigned long int, FMOD::Studio::EventInstance*>(id, event_instance));

		e = errorCheck(event_instance->start());
		if (e == 1) { return 0; }

		return id;
	}
	else
	{
		return 0;
	}
}

unsigned long int FmodWrapper::play2DEvent(const std::string& event, std::map<std::string, float> parameters)
{
	if (!audio_engine_initialized) { return 0; }

	FMOD::Studio::EventDescription* event_description = nullptr;
	int e = errorCheck(audio_engine->studio_system->getEvent(event.c_str(), &event_description));
	if (e == 1) { return 0; }

	FMOD::Studio::EventInstance* event_instance = nullptr;
	e = errorCheck(event_description->createInstance(&event_instance));
	if (e == 1) { return 0; }

	if (!parameters.empty())
	{
		for (auto it = parameters.begin(); it != parameters.end(); it++)
		{
			std::string parameter = it->first;
			float value = it->second;
			errorCheck(event_instance->setParameterByName(parameter.c_str(), value, false));
		}
	}

	if (id_system != nullptr)
	{
		unsigned long int id = id_system->getUniqueId();
		audio_engine->m_events.insert(std::pair<unsigned long int, FMOD::Studio::EventInstance*>(id, event_instance));

		e = errorCheck(event_instance->start());
		if (e == 1) { return 0; }
		
		return id;
	}
	else
	{
		return 0;
	}
}

int FmodWrapper::stopEvent(int event_id, bool allow_fades)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_events.find(event_id);

	if (find_key != audio_engine->m_events.end())
	{
		int e;

		if (allow_fades)
		{
			e = errorCheck(find_key->second->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
			if (e == 1) { return 0; }
			return 1;
		}
		else
		{
			e = errorCheck(find_key->second->stop(FMOD_STUDIO_STOP_IMMEDIATE));
			if (e == 1) { return 0; }
			return 1;
		}
	}
	else
	{
		return 0;
	}
}

int FmodWrapper::set3DAttributes(int event_id, FMOD_3D_ATTRIBUTES spatial_attributes)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_events.find(event_id);

	if (find_key != audio_engine->m_events.end())
	{
		int e;

		e = errorCheck(find_key->second->set3DAttributes(&spatial_attributes));
		if (e == 1) { return 0; }
		return 1;
	}
	else
	{
		return 0;
	}
}

int FmodWrapper::setListenerAttributes(int listener_index, FMOD_3D_ATTRIBUTES spatial_attributes)
{
	if (!audio_engine_initialized) { return 0; }

	int e;

	e = errorCheck(audio_engine->studio_system->setListenerAttributes(listener_index, &spatial_attributes));
	if (e == 1) { return 0; }
	return 1;
}

int FmodWrapper::setParameterByName(int event_id, std::string& parameter, float value)
{
	if (!audio_engine_initialized) { return 0; }

	auto find_key = audio_engine->m_events.find(event_id);

	if (find_key != audio_engine->m_events.end())
	{
		int e;
		e = errorCheck(find_key->second->setParameterByName(parameter.c_str(), value));
		if (e == 1) { return 0; }
		return 1;
	}
	else
	{
		return 0;
	}
}

int FmodWrapper::setGlobalParameterByName(std::string& parameter, float value)
{
	if (!audio_engine_initialized) { return 0; }

	int e;

	e = errorCheck(audio_engine->studio_system->setParameterByName(parameter.c_str(), value, false));
	if (e == 1) { return 0; }
	return 1;
}

int FmodWrapper::setBusPauseStatus(const std::string& bus, bool is_paused)
{
	if (!audio_engine_initialized) { return 0; }

	int e;

	FMOD::Studio::Bus* b = nullptr;
	e = errorCheck(audio_engine->studio_system->getBus(bus.c_str(), &b));
	
	bool bus_is_valid = b->isValid();

	if (bus_is_valid)
	{
		b->setPaused(is_paused);
		return 1;
	}
	else
	{
		return 0;
	}
}

int FmodWrapper::stopAllBusEvents(const std::string& bus, bool allow_fades)
{
	if (!audio_engine_initialized) { return 0; }

	int e;

	FMOD::Studio::Bus* b = nullptr;
	e = errorCheck(audio_engine->studio_system->getBus(bus.c_str(), &b));

	bool bus_is_valid = b->isValid();

	if (bus_is_valid)
	{
		int e;

		if (allow_fades)
		{
			e = errorCheck(b->stopAllEvents(FMOD_STUDIO_STOP_ALLOWFADEOUT));
			if (e == 1) { return 0; }
			return 1;
		}
		else
		{
			e = errorCheck(b->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE));
			if (e == 1) { return 0; }
			return 1;
		}
	}
	else
	{
		return 0;
	}
}

FMOD_RESULT F_CALLBACK FmodWrapper::dialogueEventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void *parameter)
{
	int e;
	auto instance = (FMOD::Studio::EventInstance*)event;
	void* user_data = nullptr;
	e = errorCheck(instance->getUserData(&user_data));

	switch (type)
	{
		case FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND:
		{
			std::cout << "Created a programmer sound" << std::endl; // Temp debug print.

			DialogueUserData dialogue_user_data = *(DialogueUserData*)user_data;
			FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES* properties = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*)parameter;
			FMOD_STUDIO_SOUND_INFO sound_info;
			e = errorCheck(audio_engine->studio_system->getSoundInfo(dialogue_user_data.line_key.c_str(), &sound_info));
			if (e == 1) { break; }

			FMOD_MODE sound_mode = FMOD_DEFAULT;
			sound_mode |= FMOD_LOOP_NORMAL;
			//sound_mode |= FMOD_CREATECOMPRESSEDSAMPLE; 
			sound_mode |= FMOD_NONBLOCKING;

			if (dialogue_user_data.is_3d)
			{
				sound_mode |= FMOD_3D;
			}

			FMOD::Sound* dialogue_sound = nullptr;
			e = errorCheck(audio_engine->core_system->createSound(sound_info.name_or_data, sound_mode, &sound_info.exinfo, &dialogue_sound)); 

			if (e == 1) { break; }

			FMOD_SOUND* cast_dialogue_sound = (FMOD_SOUND*)dialogue_sound;
			properties->sound = cast_dialogue_sound;
			properties->subsoundIndex = sound_info.subsoundindex;
		}
		break;
	
		case FMOD_STUDIO_EVENT_CALLBACK_STOPPED:
		{
			std::cout << "An event stopped" << std::endl;  // Temp debug print.	
			instance->release();			
		}
		break;

		//  When this callback happens, inform the dialogue system that the current line has finished.
		//  Maybe also add another system on the game engine side that allows for a wait before the next line is triggered for dialogue pacing purposes.
		//  Interrupting lines are their own special case, since the next line has to be triggered before the current one has been destoryed.
		case FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND: 
		{			
			std::cout << "Destroyed a programmer sound" << std::endl; // Temp debug print.	
			FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES* properties = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*)parameter;
			FMOD::Sound* cast_dialogue_sound = (FMOD::Sound*)properties->sound;
			cast_dialogue_sound->release();			
		}
		break;

		case FMOD_STUDIO_EVENT_CALLBACK_DESTROYED:
		{
			std::cout << "Destroyed an event" << std::endl; // Temp debug print.
			DialogueUserData dialogue_user_data = *(DialogueUserData*)user_data;			
			audio_engine->m_alloc_dialogue_user_data.erase(dialogue_user_data.associated_event_id);
			delete user_data;
			user_data = nullptr;
		}
		break;
	}
	return FMOD_OK;
}

unsigned long int FmodWrapper::playDialogue3D(const std::string key, DialogueMasterEvents master_event, FMOD_3D_ATTRIBUTES spatial_attributes, std::map<std::string, float> parameters)
{
	if (!audio_engine_initialized) { return 0; }

	std::string dialogue_master_event;

	switch (master_event)
	{
		case PC:
			dialogue_master_event = "event:/PC_Dialogue_Master";
			break;
		case NPC:
			dialogue_master_event = "event:/NPC_Dialogue_Master";
			break;
		default:
			return 0;
	}

	FMOD::Studio::EventDescription* dialogue_event_description = nullptr;
	int e = errorCheck(audio_engine->studio_system->getEvent(dialogue_master_event.c_str(), &dialogue_event_description));
	if (e == 1) { return 0; } 

	FMOD::Studio::EventInstance* dialogue_event_instance = nullptr;
	e = errorCheck(dialogue_event_description->createInstance(&dialogue_event_instance));
	if (e == 1) { return 0; } 

	bool is_3d = false;
	dialogue_event_description->is3D(&is_3d);

	if (is_3d)
	{
		e = errorCheck(dialogue_event_instance->set3DAttributes(&spatial_attributes));
		if (e == 1) { return 0; }
	}
	else
	{
		return 0;
	}

	if (!parameters.empty())
	{
		for (auto it = parameters.begin(); it != parameters.end(); it++)
		{
			std::string parameter = it->first;
			float value = it->second;
			errorCheck(dialogue_event_instance->setParameterByName(parameter.c_str(), value, false));
		}
	}

	if (id_system != nullptr)
	{
		unsigned long int id = id_system->getUniqueId();
		audio_engine->m_events.insert(std::pair<unsigned long int, FMOD::Studio::EventInstance*>(id, dialogue_event_instance));

		dialogue_event_instance->setCallback(dialogueEventCallback,
											 FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND |
											 FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND |
											 FMOD_STUDIO_EVENT_CALLBACK_STOPPED |
											 FMOD_STUDIO_EVENT_CALLBACK_DESTROYED);

		DialogueUserData* dialogue_user_data = new DialogueUserData();
		dialogue_user_data->is_3d = true;
		dialogue_user_data->line_key = key;
		dialogue_user_data->associated_event_id = id;
		audio_engine->m_alloc_dialogue_user_data.insert(std::pair<unsigned long int, DialogueUserData*>(id, dialogue_user_data));

		e = errorCheck(dialogue_event_instance->setUserData(dialogue_user_data));
		if (e == 1) 
		{
			audio_engine->m_alloc_dialogue_user_data.erase(id);
			delete dialogue_user_data;
			dialogue_user_data = nullptr;
			return 0; 
		}

		e = errorCheck(dialogue_event_instance->start());
		if (e == 1) 
		{
			audio_engine->m_alloc_dialogue_user_data.erase(id);
			delete dialogue_user_data;
			dialogue_user_data = nullptr;
			return 0; 
		}

		return id;
	}
	else
	{
		return 0;
	}
}

unsigned long int FmodWrapper::playDialogue2D(const std::string key, DialogueMasterEvents master_event, std::map<std::string, float> parameters)
{	
	if (!audio_engine_initialized) { return 0; }

	std::string dialogue_master_event;

	switch (master_event)
	{
	case PC:
		dialogue_master_event = "event:/PC_Dialogue_Master";
		break;
	case NPC:
		dialogue_master_event = "event:/NPC_Dialogue_Master";
		break;
	default:
		return 0;
	}

	FMOD::Studio::EventDescription* dialogue_event_description = nullptr;
	int e = errorCheck(audio_engine->studio_system->getEvent(dialogue_master_event.c_str(), &dialogue_event_description));
	if (e == 1) { return 0; }

	FMOD::Studio::EventInstance* dialogue_event_instance = nullptr;
	e = errorCheck(dialogue_event_description->createInstance(&dialogue_event_instance));
	if (e == 1) { return 0; }

	if (!parameters.empty())
	{
		for (auto it = parameters.begin(); it != parameters.end(); it++)
		{
			std::string parameter = it->first;
			float value = it->second;
			errorCheck(dialogue_event_instance->setParameterByName(parameter.c_str(), value, false));
		}
	}

	if (id_system != nullptr)
	{
		unsigned long int id = id_system->getUniqueId();
		audio_engine->m_events.insert(std::pair<unsigned long int, FMOD::Studio::EventInstance*>(id, dialogue_event_instance));

		dialogue_event_instance->setCallback(dialogueEventCallback, 
								             FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND | 
											 FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND | 
											 FMOD_STUDIO_EVENT_CALLBACK_STOPPED |
											 FMOD_STUDIO_EVENT_CALLBACK_DESTROYED);

		DialogueUserData* dialogue_user_data = new DialogueUserData();
		dialogue_user_data->is_3d = false;
		dialogue_user_data->line_key = key;
		dialogue_user_data->associated_event_id = id;
		audio_engine->m_alloc_dialogue_user_data.insert(std::pair<unsigned long int, DialogueUserData*>(id, dialogue_user_data));

		e = errorCheck(dialogue_event_instance->setUserData(dialogue_user_data));
		if (e == 1)
		{
			audio_engine->m_alloc_dialogue_user_data.erase(id);
			delete dialogue_user_data;
			dialogue_user_data = nullptr;
			return 0;
		}

		e = errorCheck(dialogue_event_instance->start());
		if (e == 1)
		{
			audio_engine->m_alloc_dialogue_user_data.erase(id);
			delete dialogue_user_data;
			dialogue_user_data = nullptr;
			return 0;
		}
		return id;
	}
	else
	{
		return 0;
	}	
}

