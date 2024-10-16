This project was designed with linux first compatibility in mind. The underlying GUI library (WxWidgets) is cross platform. However, it requires minor code adjustments first. The project in its current state WILL NOT work on windows.

The 2 executable files attached are for linux only. However, they depend on the WxWidgets shared libraries to be installed. This will depend on your distribution. However, I have attached a small BASH script to install the WxWidgets shared libraries depending on your distro.

If you would like to build the project from source, then make sure you have the ASIO and WxWidgets libraries installed on your system. Also, Cmake and g++ are required to build the project. Then, execute the below command while in the root directory of the project

cmake -S ./src -B ./build && cd ./build && make && cd ..

There will now be 2 executables inside the 'build' folder: SERVER_EXECUTABLE_CLI and CLIENT_EXECUTABLE_GUI. The SERVER_EXECUTABLE_CLI must be run inside a terminal. However, the CLIENT_EXECUTABLE_GUI can be launched directly without a terminal.

A maximum of 24 concurrent users is supported.

IMPORTANT NOTE: THE SERVER MUST BE OPENED BEFORE THE CLIENT INSTANCES ARE OPENED. IF THE SERVER IS DOWN, THEN THE GUI CLIENTS WILL FAIL TO OPEN.

Lastly, the network architecture is heavily inspired by David Bahr's (Javidx9) networking framework
for his olcPixelGameEngine.
