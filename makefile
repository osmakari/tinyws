# Use this if you don't have pthreads
# tinywsdemo: test/demo.c src/*.c src/*.h
# 	 gcc src/*.c test/demo.c -g -o test/bin/tinywsdemo
# Use this if you have pthreads
tinywsdemo: test/demo.c src/*.c src/*.h
	gcc src/*.c test/demo.c -lpthread -g -o test/bin/tinywsdemo