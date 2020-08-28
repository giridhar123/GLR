<<<<<<< HEAD
bison:
	bison -d parser.y
	
flex:
	flex lex.l
	
out:
	gcc *.c -o glr -lfl -pthread

win: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -lfl -pthread

mac: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -ll -pthread

clean : 
=======
bison:
	bison -d parser.y
	
flex:
	flex lex.l
	
out:
	gcc *.c -o glr -lfl -pthread

win: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -lfl -pthread

mac: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -ll -pthread

clean : 
>>>>>>> bdf7bc6f14ce78576df06bb315f6275de936a165
	rm parser.tab.* lex.yy.c glr