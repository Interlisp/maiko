/* @(#) find-dispatch.lex Version 1.2 (4/7/89). copyright envos & Fuji Xerox  */

REG	"%"[oilg][0-9]
REGOFF	"%"[oilg][0-9]\+\-0x[0-9]
LABEL	L[0-9]+
LDUB	"\tldub\t["{REG}"],"{REG}
LDUB2	"\tldub\t["{REGOFF}"],"{REG}

%%

{LDUB2}\n\tcmp\t{REG}",255\n\tbgu\t"{LABEL}\n\t\sll\t{REG}",2,"{REG}\n\tset\t	{	char x;
	printf("char *dispatch_label = \"");
	loop: x=input();
	if(x == ','){ printf("\";\n"); return;}
	output(x); goto loop;}

.*\n	{}
