#指定当前目录下所有文件生成可执行文件 包含该目录下所有文件。
aux_source_directory(. ENCRYPT_LIST)

add_library(myencryption STATIC ${ENCRYPT_LIST})

target_include_directories(myencryption PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(myencryption PUBLIC ssl crypto)