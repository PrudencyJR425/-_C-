#指定当前目录下所有文件生成可执行文件 包含该目录下所有文件。
aux_source_directory(. SRC_LIST)
#如果还有别的目录需要生成可执行文件:如本文件夹下还有db文件夹:
#aux_source_directory(.db DB_LIST)

#指定生成可执行文件
add_executable(ChatClient ${SRC_LIST})
#add_executable(ChatServer ${SRC_LIST}${DB_LIST})

#指定可执行文件链接时依赖的库文件 读写两个线程
target_link_libraries(ChatClient pthread)