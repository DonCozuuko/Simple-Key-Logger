main:
	gcc klog.c -o d -lws2_32 && gcc server.c -o s -lws2_32

# main :
# 	gcc key.c -o c -lpsapi