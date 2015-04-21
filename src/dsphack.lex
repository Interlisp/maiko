/* @(#) disphack.lex Version 1.5 (11/2/88). copyright envos & Fuji Xerox  */

REG	"%"[oilg][0-9]
LABEL	L[0-9]+
COMPUTE	"\tcall\t_compute_dispatch_table,0\n	nop\n\tmov\t%o0,"
DISPATCH "\tmov\t"{REG}",%o0\n\tcall\t_fast_dispatcher,2\n\tnop\n"
HEXD	0x[0-3]
ADDX	"\tadd\t"{REG}","{HEXD}","{REG}
LDUB	"\tldub\t["{REG}"],"{REG}
LDUBm1	"\tldub\t["{REG}"+-0x1],%o1"
MOV	"\tmov\t"{REG}",%o1"

%%
	extern char *dispatch_label;


{COMPUTE}	printf(" set %s,", dispatch_label);

{ADDX}\n{LDUBm1}\n{DISPATCH} {
	if (    memcmp(yytext + 5, yytext + 13, 3)
	    ||  memcmp(yytext + 5, yytext + 24, 3)) {REJECT};

	printf(" ldub [%.3s+%.3s+-0x1],%%o1\n",yytext+5, yytext+9);
	printf(" sll %%o1,2,%%o1\n");
	printf(" ld [%%o1+%.3s],%%o1\n",yytext+43);
	printf(" jmp %%o1\n");
	printf("%.16s\n", yytext);
}



{DISPATCH}	{
	printf(" sll %%o1,2,%%o1\n");
	printf(" ld [%%o1+%%%.2s],%%o1\n",yytext + 6);
	printf(" jmp %%o1\n");
	printf(" nop\n");
}

{ADDX}\n{LDUB}	{
	if (   !memcmp(yytext + 5, yytext + 13, 3)
	    && !memcmp(yytext + 5, yytext + 24, 3)
	    &&  memcmp(yytext + 5, yytext + 29, 3)) {
		printf(" ldub [%.3s+%.3s],%.3s\n%.16s\n",
			 yytext+5,yytext+9,yytext+29,yytext);
	} else
		printf("%s", yytext);
}


{LDUB}\n\tcmp\t{REG}",255\n\tbgu\t"{LABEL}\n\t\sll\t{REG}",2,"{REG}\n\tset\t{LABEL}","	{
	if (memcmp(yytext+yyleng-strlen(dispatch_label)-1,
			dispatch_label,
			strlen(dispatch_label))) {
		fprintf(stderr, "Label in dispatch changed. Edit disphack.lex");
		fprintf(stderr, " and change '%s' to label", dispatch_label);
		fprintf(stderr, " in last line of \n\n%s\n\n and retry!\n", yytext);
		exit(-1);
	} else if (memcmp(yytext + 12, yytext + 21, 3)) {
		fprintf(stderr, "Odd sequence %s\n", yytext);
	};
	REJECT;
};
