CC = g++
src = $(wildcard ./*.c ./util/*.c ./User/*.c ./Lthread/Llock.c ./LSocket/*.c ./LSocket/UDP/*.c ./LReactor/*.c ./LMysql/*.c ./Llib/*.c ./common_component/rapidjson/*.c)
obj = $(patsubst %.c, %.o, $(src))
target = server
$(target): $(obj)
	$(CC) $(obj) -o $(target) -std=c++11 -pthread -L/usr/lib/mysql -lmysqlclient
%.o:%.c
	$(CC) -c $< -o $@ -std=c++11 -L/usr/lib/mysql -lmysqlclient

.PHONY:clean

clean:
	rm $(obj) $(target) -f
