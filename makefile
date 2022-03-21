src = $(wildcard ./*.c)
obj = $(patsubst %.c, %.o, $(src))
target = server
$(target): $(obj)
	g++ $(obj) -o $(target) -std=c++11 -pthread -L/usr/lib/mysql -lmysqlclient
%.o:%.c
	g++ -c $< -o $@ -std=c++11 -L/usr/lib/mysql -lmysqlclient

.PHONY:clean

clean:
	rm $(obj) $(target) -f
