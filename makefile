CC = /opt/linaro-armv7ahf-2019.02-gcc7.4/bin/arm-linux-gnueabihf-gcc
CFALGS = -Wall -g
TARGET = timestamp_db
OBJS = timestamp_db.o test_db.o
all: ${TARGET}
	sudo cp ${TARGET} /mnt/share_folder/data_base/${TARGET}
timestamp_db: ${OBJS}
	${CC} ${CFALGS} ${OBJS} -pthread -o ${TARGET}
timestamp_db.o: timestamp_db.c timestamp_db.h
	${CC} ${CFLAGS} -c timestamp_db.c
test_db.o: test_db.c
	${CC} ${CFALGS} -c test_db.c

clean:
	rm *.o

# /opt/linaro-armv7ahf-2019.02-gcc7.4/bin/arm-linux-gnueabihf-gcc