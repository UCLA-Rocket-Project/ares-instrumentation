CC=gcc
CFLAGS= -ggdb -Wall -std=gnu99  -lpthread
EXEC=serial_tool
OBJS= *.c

all:
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $$(mysql_config --cflags --libs) -DUSE_MYSQL=1
	#$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) -I/usr/local/mysql/include -L/usr/local/mysql/lib -lmysqlclient  -DUSE_MYSQL=1

without-mysql:
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) -DUSE_MYSQL=0

install:
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $$(mysql_config --cflags --libs) -DUSE_MYSQL=1
	chmod +x $(EXEC)
	sudo mv $(EXEC) /usr/local/bin/
clean:
	rm -r $(EXEC)*
