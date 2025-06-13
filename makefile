main:
	gcc klog.c -o gg -lws2_32 -lpsapi && gcc server.c -o server -lws2_32
