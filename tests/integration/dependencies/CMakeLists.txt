cmake_minimum_required(VERSION 3.21)
project(mimir-integration)

##############################################################
# Dependencies
##############################################################

add_subdirectory(abseil)
add_subdirectory(boost)
add_subdirectory(fmt)
add_subdirectory(loki)
add_subdirectory(nauty)
add_subdirectory(mimir)
