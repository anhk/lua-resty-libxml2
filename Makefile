

#LUAINC= /usr/include/lua5.2/
LUAINC= /usr/local/LuaJit/LuaJIT-2.1.0-beta2/include/luajit-2.1/

INCS= -I$(LUAINC) -I /usr/include/libxml2/

CFLAGS= $(INCS) -O0 -g -fPIC

all: libxml2.so lua-libxml2.so


lua-libxml2.so: lua-libxml2.c
	$(CC) $(CFLAGS) -o $@ -shared $^ -lxml2

libxml2.so: lua-libxml2.so
	cp -af $< $@

clean:
	rm -fr lua-libxml2.so
