one:
	gcc smtpmail.c -o smtp
	./smtp 8000
two:
	gcc popserver.c -o pop
	./pop 61000
three:
	gcc mailclient.c -o cli
	./cli 127.0.0.1 8000 61000