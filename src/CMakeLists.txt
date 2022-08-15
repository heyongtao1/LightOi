

aux_source_directory(common_component/base64 LIBS_BASE64)
aux_source_directory(Logger LIBS_LOGGER)
# 生成静态库
add_library(base64 STATIC ${LIBS_BASE64}) 
add_library(logger STATIC ${LIBS_LOGGER}) 
