one:
	gcc smtpmail.c -o smtp
	./smtp 8001
two:
	gcc popserver.c -o pop
	./pop 6051
three:
	gcc mailclient.c -o cli.exe
	./cli.exe 127.0.0.1 8001 6051
clean:
	rm -f cli.exe pop smtp