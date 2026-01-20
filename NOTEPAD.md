
# TODO INPUT:
- (DONE) Add channel to event system so front end can receive events 
- (DONE) Make graceful shutdown function for the EventSystem
- (DONE) Make frontend get a receiver from event system inner channel so it can read inputs
- (DONE) make creation of InputDevices so that user can choose what type 
of peripherals to transform from peripherals list
- (DONE) make core logic of the event system:
- (DONE) Make polling logic and read from eventX binary files
- (DONE) Translate raw values to event compatible data structure
- (DONE) logical gamepad keypad (up/down/left/right) are done, make the analog version (LINUX)
--- 
- make gamepad/keyboard/mouse hotpluggable during runtime without restart
- Make peripherals/inputdevice and Event System Polling WINDOWS/APPLE compatible
- Get mouse/gamepad coordinates on the screen instead of only the relative movements it made on event, for menus/UI it is useful 
    - LINUX   : X11/Wayland
    - WINDOWS : win32
    - APPLE   : cocoa
    - (RGFW already has this  abstracted)

# TODO RENDERING:
- after a decent event input/event system start rendering system
---
    - setup RGFW with window init and OpenGL context
    - incorporate SAE Event System into RGFW

# ENTITY SYSTEM:
- after a decent rendering system start the entity system
