# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.22)

project(ChatApp)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Find packages
#TO COMPILE WX-WIDGETS FROM SOURCE FOR THE STATIC LIBRARIES

find_package(wxWidgets REQUIRED COMPONENTS gl aui core base stc OPTIONAL_COMPONENTS net)
set(wxWidgets_USE_STATIC TRUE)
set(CMAKE_BUILD_TYPE Static)
set(wxWidgets_CONFIGURATION Static)

# Include the wxWidgets use file
include(${wxWidgets_USE_FILE})

# Add executable
add_executable(CLIENT_EXECUTABLE_GUI App.cpp MainFrame.cpp CLIENT_INTERFACE.cpp net_client.h
net_common.h net_connection.h net_message.h net_server.h net_tsqueue.h net_All.h timer.cpp)
add_executable(SERVER_EXECUTABLE_CLI SimpleServer.cpp net_client.h net_common.h net_connection.h net_message.h net_server.h net_tsqueue.h net_All.h)
#add_executable(MESSAGING_TEST_CLIENT SimpleClient.cpp net_client.h net_common.h net_connection.h
#net_message.h net_server.h net_tsqueue.h net_All.h)
#target_compile_options(MESSAGING_TEST_CLIENT PRIVATE -g)
target_compile_options(CLIENT_EXECUTABLE_GUI PRIVATE -g)
target_compile_options(SERVER_EXECUTABLE_CLI PRIVATE -O2)


# Include Asio headers
target_include_directories(CLIENT_EXECUTABLE_GUI PRIVATE asio/asio/include)
target_include_directories(SERVER_EXECUTABLE_CLI PRIVATE asio/asio/include)
#target_include_directories(MESSAGING_TEST_CLIENT PRIVATE asio/asio/include)

# Link with appropriate system libraries (e.g., pthread)
find_package(Threads REQUIRED)
target_link_libraries(CLIENT_EXECUTABLE_GUI Threads::Threads)
target_link_libraries(CLIENT_EXECUTABLE_GUI ${wxWidgets_LIBRARIES} pthread)
target_link_libraries(SERVER_EXECUTABLE_CLI Threads::Threads)
#target_link_libraries(MESSAGING_TEST_CLIENT Threads::Threads)
#target_link_libraries(MESSAGING_TEST_CLIENT ${wxWidgets_LIBRARIES} pthread)


