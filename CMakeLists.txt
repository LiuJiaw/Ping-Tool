cmake_minimum_required(VERSION 3.5)

project(Ping)

set(SOURCE_FILES 
	main.cpp
	ping.cpp
	ping.h
        )

add_executable(Ping ${SOURCE_FILES})
