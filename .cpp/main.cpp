MIT License
Copyright (c) 2020 Ville Ojala

#include "main.h"

FmodWrapper fmod_wrapper;

// Debug testing all the different functionalities of the wrapper.
// In order to run the tests replace the FMOD Studio project specific fields with your own data.

int main()
{	
	fmod_wrapper.initializeAudioEngine();
	int r;

	 // 1. Load the metadata of the needed banks.

	std::string sample_bank_path = "D:/FmodTestProject/Build/Desktop/Samples.bank";
	printInfo("Loading the sample bank metadata.");
	r = fmod_wrapper.loadBank(sample_bank_path, false);
	if (r == 1) { printInfo("Sample bank metadata was succesfully loaded!"); }
	
	std::string audio_table_bank_path = "D:/FmodTestProject/Build/Desktop/Voiceovers_EN.bank";
	printInfo("\nLoading voiceover bank metadata.");
	r = fmod_wrapper.loadBank(audio_table_bank_path, false);
	if (r == 1) { printInfo("Voiceover bank metadata was succesfully loaded!"); }


	// 2. Load the sample data of the needed banks.
	
	printInfo("\nLoading the sample bank's sample data.");
	fmod_wrapper.loadSampleData(sample_bank_path);
	if (r == 1) { printInfo("Sample bank's sample data was succesfully loaded!"); }
	
	printInfo("\nLoading voiceover bank's sample data.");
	fmod_wrapper.loadSampleData(audio_table_bank_path);
	if (r == 1) { printInfo("Voiceover bank's sample data was succesfully loaded!"); }

	printInfo("\nPress enter to continue ->");
	std::cin.get();
	fmod_wrapper.callUpdate();


	// 3. Test 2D dialogue
	
	std::string test_key_2D = "gloves";
	fmod_wrapper.playDialogue2D(test_key_2D, FmodWrapper::DialogueMasterEvents::PC);
	fmod_wrapper.callUpdate();
	std::cin.get();
	

	// 4. Test 3D dialogue

	FMOD_3D_ATTRIBUTES vo_attributes;
	vo_attributes.position = { 3.0f, 0.0f, 0.0f };
	vo_attributes.velocity = { 0.0f, 0.0f, 0.0f };
	vo_attributes.forward = { 0.0f, 0.0f, 1.0f };
	vo_attributes.up = { 0.0f, 1.0f, 0.0f };

	std::string test_key_3D = "gloves";
	fmod_wrapper.playDialogue3D(test_key_3D, FmodWrapper::DialogueMasterEvents::PC, vo_attributes);
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();

	
	// 5. Start a looping 3D event (set local parameter to a non-default value).
	
	std::string primary_event = "event:/Primary";
	std::map<std::string, float> initial_parameters;
	std::string parameter_local = "LocalParameter";
	float parameter_local_value = 0.1f;
	initial_parameters[parameter_local] = parameter_local_value;
	
	FMOD_3D_ATTRIBUTES default_attributes;
	default_attributes.position = { 0.0f, 0.0f, 0.0f };
	default_attributes.velocity = { 0.0f, 0.0f, 0.0f };
	default_attributes.forward = { 0.0f, 0.0f, 1.0f };
	default_attributes.up = { 0.0f, 1.0f, 0.0f };
	
	int primary_unique_id;
	printInfo("Starting a 3D event");
	primary_unique_id = fmod_wrapper.play3DEvent(primary_event, default_attributes, initial_parameters);
	if (primary_unique_id != 0) { printInfo("3D event succesfully started!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 6. Change the local parameter value. Retrieve the correct event instance by using the returned unique ID.

	printInfo("Changing the value of the local parameter for the previously started 3D event instance.");
	r = fmod_wrapper.setParameterByName(primary_unique_id, parameter_local, 1.0f);
	if (r == 1) { printInfo("Value for the local parameter succesfully changed!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 7. Changing a global parameter value.

	std::string parameter_global = "GlobalParameter";
	float parameter_global_value = 1.0f;
	printInfo("Changing the value for a global parameter.");
	r = fmod_wrapper.setGlobalParameterByName(parameter_global, parameter_global_value);
	if (r == 1) { printInfo("Global parameter value succesfully changed!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 8. Changing the 3D attributes of the playing 3D event. etrieve the correct event instance by using the stored unique ID.

	FMOD_3D_ATTRIBUTES modified_attributes;
	modified_attributes.position = { 3.0f, 0.0f, 0.0f };
	modified_attributes.velocity = { 0.0f, 0.0f, 0.0f };
	modified_attributes.forward = { 0.0f, 0.0f, 1.0f };
	modified_attributes.up = { 0.0f, 1.0f, 0.0f };
	
	printInfo("Changing the 3D attributes of the playing 3D event.");
	r = fmod_wrapper.set3DAttributes(primary_unique_id, modified_attributes);
	if (r == 1) { printInfo("Event 3D attributes succesfully changed!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 9. Pausing and unpausing the master bus (i.e. all the events routed to it).

	std::string master_bus = "bus:/";
	printInfo("Pausing the master bus.");
	r = fmod_wrapper.setBusPauseStatus(master_bus, true);
	if (r == 1) { printInfo("Master bus succesfully paused!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();

	printInfo("Unpausing the master bus.");
	r = fmod_wrapper.setBusPauseStatus(master_bus, false);
	if (r == 1) { printInfo("Master bus succesfully unpaused!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 10. Stopping the looping 3D event.

	printInfo("Stopping the looping 3D event with a fade out");
	r = fmod_wrapper.stopEvent(primary_unique_id, true);
	if (r == 1) { printInfo("Event succesfully stopped!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();

	fmod_wrapper.callUpdate();


	// 11. Starting a new looping 2D event. No optional parameters provided.

	std::string secondary_event = "event:/Secondary";
	int secondary_unique_id;
	printInfo("Starting a 2D event");
	secondary_unique_id = fmod_wrapper.play2DEvent(secondary_event);
	if (secondary_unique_id != 0) { printInfo("Succesfully started a 2D event instance!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 12. Try to access the already deleted 3D event instance with its unique ID, i.e. test the error handling.
	
	printInfo("Trying to access a unique ID for already deleted event instance.");
	r = fmod_wrapper.setParameterByName(primary_unique_id, parameter_local, 0.5f);
	if (r == 0) { printInfo("Invalid unique ID access handled correctly!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 13. Stop all events routed to the master bus.
	
	printInfo("Stop all events routed to the master bus.");
	r = fmod_wrapper.stopAllBusEvents(master_bus, true);
	if (r == 1) { printInfo("Events to the master bus succesfully stopped!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 14. Unload sample bank's sample data.

	printInfo("Unloading sample bank's sample data.");
	r = fmod_wrapper.unloadSampleData(sample_bank_path);
	if (r == 1) { printInfo("Sample bank's sample data succesfully unloaded!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 15. Unload sample bank's metadata.

	printInfo("Unloading sample bank's metadata.");
	r = fmod_wrapper.unloadBank(sample_bank_path);
	if (r == 1) { printInfo("Sample bank's metadata succesfully unloaded!"); }
	fmod_wrapper.callUpdate();
	printInfo("Press enter to continue ->");
	std::cin.get();


	// 16. Shut down the audio engine

	printInfo("Shutting down the audio engine.");
	fmod_wrapper.shutDownAudioEngine();
		
	return 0;
}

