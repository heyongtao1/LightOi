src = $(wildcard ./*.c)
obj = $(patsubst %.c, %.o, $(src))
target = server
$(target): $(obj)
	g++ $(obj) -o $(target) -std=c++11 -pthread
%.o:%.c
	g++ -c $< -o $@ -std=c++11

.PHONY:clean

clean:
	rm $(obj) $(target) -f
