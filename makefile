one:
	gcc smtpmail.c -o smtp
	./smtp 40000

two:
	gcc mailclient.c -o cli
	./cli 127.0.0.1 40000 50000