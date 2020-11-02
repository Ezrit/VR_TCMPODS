cmake_minimum_required(VERSION 3.12)

project(NDD_VR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    message(STATUS ">>> Windows")
    set(GLEW_DIR C:/Libs/GLEW_2.1.0 CACHE PATH "Path to GLEW directory")
    set(GLEW_INCLUDE_DIR ${GLEW_DIR}/include)
    set(GLEW_LIBRARY_DIR ${GLEW_DIR}/lib/Release/Win32)
    set(GLEW_LIBRARIES glew32)

    set(GSTREAMER_BASE_DIR C:/gstreamer/1.0 CACHE PATH "Basepath to GStreamer directory")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bit
        set(GSTREAMER_DIR ${GSTREAMER_BASE_DIR}/x86_64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 bit
        set(GSTREAMER_DIR ${GSTREAMER_BASE_DIR}/x86)
    endif()
    set(GSTREAMER_INCLUDE_DIRS ${GSTREAMER_DIR}/include/gstreamer-1.0 ${GSTREAMER_DIR}/lib/gstreamer-1.0/include ${GSTREAMER_DIR}/include/glib-2.0 ${GSTREAMER_DIR}/lib/glib-2.0/include)
    set(GSTREAMER_LIBDIR ${GSTREAMER_DIR}/lib/)
    file( GLOB gst_lib_files ${GSTREAMER_DIR}/lib/*.lib)
    set(GSTREAMER_LIBRARIES ${gst_lib_files})

    add_definitions(-DOPENVR_BUILD_STATIC)

elseif(UNIX)
    message(STATUS ">>> UNIX")
    set(GLEW_LIBRARIES "GLEW")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GSTREAMER gstreamer-1.0 gstreamer-net-1.0)
endif()

set(OpenVR_DIR "/local/muehlhausen/Git/openvr/install" CACHE PATH "OpenVR directory")
set(OpenVR_INCLUDE_DIR ${OpenVR_DIR}/include/openvr)
set(OpenVR_LIBRARY_DIR ${OpenVR_DIR}/lib)
set(OpenVR_LIBRARY "openvr_api")


set(STB_DIR "" CACHE PATH "Path to stb directory")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)



set(MSI_VR_FRAMEWORK_SRC ${SRC_DIR}/vrcontext.cpp 
               ${SRC_DIR}/application.cpp 
               ${SRC_DIR}/nddapplication.cpp
               ${SRC_DIR}/inputmanager.cpp 
               ${SRC_DIR}/mouse.cpp 
               ${SRC_DIR}/shader.cpp 
               ${SRC_DIR}/globject.cpp 
               ${SRC_DIR}/quad.cpp 
               ${SRC_DIR}/odssphereobject.cpp 
               ${SRC_DIR}/texture.cpp 
               ${SRC_DIR}/videotexture.cpp 
               ${SRC_DIR}/videosynchronizer.cpp 
               ${SRC_DIR}/renderpass.cpp)

add_library(NDD_VR_FRAMEWORK ${MSI_VR_FRAMEWORK_SRC})
target_include_directories(NDD_VR_FRAMEWORK PUBLIC ${INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLM_INCLUDE_DIRS} ${STB_DIR} ${GSTREAMER_INCLUDE_DIRS} ${OpenVR_INCLUDE_DIR})
target_link_directories(NDD_VR_FRAMEWORK PUBLIC ${GLEW_LIBRARY_DIR} ${GSTREAMER_LIBDIR} ${OpenVR_LIBRARY_DIR})
target_link_libraries(NDD_VR_FRAMEWORK ${OPENGL_LIBRARY} glfw ${GLEW_LIBRARIES} ${GSTREAMER_LIBRARIES} ${OpenVR_LIBRARY})


add_executable(NDD_VR main.cpp)
target_include_directories(NDD_VR PUBLIC ${INCLUDE_DIR})
target_link_libraries(NDD_VR MSI_VR_FRAMEWORK)
