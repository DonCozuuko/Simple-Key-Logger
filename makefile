main:
	gcc ./src/klog.c -o ggs -lws2_32 -lpsapi && gcc ./src/server.c -o server -lws2_32
