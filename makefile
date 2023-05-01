
shell:shell.c
		gcc -o shell shell.c -lreadline

clean:
	rm -f shell

debug:
	gcc -g -o shellt shell.c -lreadline