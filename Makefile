#ppp
#-------------变量定义
TARGET=main
CC=gcc
#把不同的路径赋给相应的变量
DIR_DEBUG=./debug
DIR_SRC=./source
DIR_IND=./include 

#把./source中的所有.c文件拿出来赋给变量SRC_PATH，包括.c文件的路径
SRC_PATH=$(wildcard ${DIR_SRC}/*.c)

#去掉SRC的路径，然后把所有.c文件后缀名，改为.o，最后把这些.o文件赋给变量
OBJ = $(patsubst %.c,%.o,$(notdir ${SRC_PATH}))

#把debug文件夹下的所有.o文件赋给变量
OBJ_PATH=$(wildcard ${DIR_DEBUG}/*.o)

${TARGET}:${OBJ}
	${CC} -o ${TARGET} ${OBJ_PATH}
${OBJ}:
	${CC} -c ${SRC_PATH} -I ${DIR_IND}

#使用shell函数，建立debug目录
	$(shell if [ -d $(DIR_DEBUG) ];then echo "exit";else mkdir debug;fi)
	mv  ${OBJ} ${DIR_DEBUG}/
clean:
	rm -rf ${TARGET} debug
all:
	@echo ${SRC_PATH}
	@echo ${OBJ}
	@echo ${OBJ_PATH}
