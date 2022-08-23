cmake_minimum_required (VERSION 2.8)
 
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
 
project (server)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/Logger)
include_directories(${PROJECT_SOURCE_DIR}/include/http)
include_directories(${PROJECT_SOURCE_DIR}/include/LMysql)
include_directories(${PROJECT_SOURCE_DIR}/include/app)
include_directories(${PROJECT_SOURCE_DIR}/include/LReactor)
include_directories(${PROJECT_SOURCE_DIR}/include/LSocket)
include_directories(${PROJECT_SOURCE_DIR}/include/LTread)
include_directories(${PROJECT_SOURCE_DIR}/include/LJob)
include_directories(${PROJECT_SOURCE_DIR}/include/common_component)
include_directories(${PROJECT_SOURCE_DIR}/include/common_component/base64)
include_directories(${PROJECT_SOURCE_DIR}/include/common_component/debug)
include_directories(${PROJECT_SOURCE_DIR}/include/common_component/rapidjson)

if (ENABLE_MYSQL)
    # 设置mysql的cmake模块
    SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    # cmake模块名为FindMySQL.cmake，故查找时候是MySQL
    FIND_PACKAGE(MySQL)
    include_directories(${MySQL_INCLUDE_DIR})

    add_definitions(" -DENABLE_MYSQL" )
    message(STATUS "ENABLE_MYSQL")
endif()

# 添加当前目录下所有的源文件
aux_source_directory(. ALL_SRCS)

# 添加子目录
add_subdirectory(src)

# 添加当前目录下所有的源文件	
if (ENABLE_MYSQL)				
    aux_source_directory(src/LMysql SRCS_MYSQL)
    aux_source_directory(src/app SRCS_APP)
else ()
    set(SRCS_MYSQL)
    set(SRCS_APP)
endif()	

aux_source_directory(src/LReactor SRCS_REACTOR)	
aux_source_directory(src/LSocket SRCS_SOCKET)
aux_source_directory(src/LJob SRCS_JOB)				

# 指定生成目标
add_executable(${PROJECT_NAME} ${SRCS_MYSQL} ${SRCS_APP} ${SRCS_SOCKET} ${SRCS_REACTOR} ${SRCS_JOB} ${ALL_SRCS})	

if (ENABLE_MYSQL)
    # 链接mysql动态链接库
    LINK_DIRECTORIES(${MySQL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} -lmysqlclient)
endif()

# 链接base64库
target_link_libraries(${PROJECT_NAME} base64)
target_link_libraries(${PROJECT_NAME} logger)
