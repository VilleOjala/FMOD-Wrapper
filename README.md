# FMOD-Wrapper

This C++ wrapper serves a starting point for integrating FMOD Studio with a game engine for which no official integration exists (i.e other than Unity or Unreal Engine). The wrapper provides an access to the most commonly needed features, such as: 

- Playing and stopping FMOD Studio events and mixer snapshots
- Updating positional data for listeners and event instances
- Setting and updating local and global parameter data for event instances
- Loading and unloading bank metadata / sample data  
- Pausing, unpausing and stopping events routed to specific mixer busses, e.g. for pause menu implementation purposes.
- Programmer sound / audio table hookup for implementing a localized dialogue system 

 Third party dependencies: 
 - FMOD Studio API version 2.01.04 (Copyright (c) Firelight Technologies, Pty, Ltd, 2011-2020)
