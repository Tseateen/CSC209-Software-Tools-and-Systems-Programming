all : simple die large server

simple : simple.o cgi.o
	gcc -Wall -g -o $@ $^

die : die.o 
	gcc -Wall -g -o $@ $^

large : large.o cgi.o
	gcc -Wall -g -o $@ $^

server : server.o
	gcc -Wall -g -o $@ $^

%.o : %.c 
	gcc -Wall -g -c $<

test_msg1 : server
	./server msg1 > server_msg1.txt
	diff expect_msg1.txt server_msg1.txt
	rm server_msg1.txt
	
test_msg2 : server
	./server msg2 > server_msg2.txt
	diff expect_msg2.txt server_msg2.txt
	rm server_msg2.txt
	
test_msg3 : server
	./server msg3 > server_msg3.txt
	diff expect_msg3.txt server_msg3.txt
	rm server_msg3.txt
	
test_real : server
	./server realmessages > server_real.txt
	diff expect_real.txt server_real.txt
	rm server_real.txt

test_404 : server
	./server msg404 > server_404.txt
	diff expect_404.txt server_404.txt
	rm server_404.txt

test_bad_format : server
	./server msg_bad > server_bad.txt
	diff expect_bad.txt server_bad.txt
	rm server_bad.txt

tests : 
	./server msg1 > server_msg1.txt
	diff expect_msg1.txt server_msg1.txt
	./server msg2 > server_msg2.txt
	diff expect_msg2.txt server_msg2.txt
	./server msg3 > server_msg3.txt
	diff expect_msg3.txt server_msg3.txt
	./server realmessages > server_real.txt
	diff expect_real.txt server_real.txt
	./server msg404 > server_404.txt
	diff expect_404.txt server_404.txt
	./server msg_bad > server_bad.txt
	diff expect_bad.txt server_bad.txt
	rm server_msg1.txt server_msg2.txt server_msg3.txt server_real.txt server_404.txt server_bad.txt

clean :
	rm *.o simple die large server

clean_test : 
	rm server_msg1.txt server_msg2.txt server_msg3.txt server_real.txt server_404.txt server_bad.txt