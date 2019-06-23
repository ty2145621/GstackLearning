#project name
PROJECT(Gstack)
CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
#head file path
INCLUDE_DIRECTORIES(
        ./
)
#source directory
AUX_SOURCE_DIRECTORY(./ DIR_SRCS)
#set environment variable
SET(MATH
        ${DIR_SRCS}
        )
#add executable file
ADD_EXECUTABLE(gstack ./${MATH})
#add link library
#TARGET_LINK_LIBRARIES(./${LIBRARIES})

