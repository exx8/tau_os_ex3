cmake_minimum_required(VERSION 3.17)
project(tau_os_ex3 C)

set(CMAKE_C_STANDARD 11)

add_executable(tau_os_ex3  message_slot.c message_slot.h  message_sender.c message_reader.c)
include_directories(/usr/src/linux-headers-5.4.0-56-generic/include)
include_directories(usr/src/linux-headers-5.4.0-56-generic/arch/ia64/include)