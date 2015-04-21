%%
^[#][Ee][Nn][Dd][Ii][Ff][ \t]+[0-9,a-z,A-Z \t|_&]+$	{printf("#endif /* %s */\n", &yytext[7]);};
^[#][Ee][Ll][Ss][Ee][ \t]+[0-9,a-z,A-Z \t|_&]+$		{printf("#else /* %s */\n", &yytext[6]);};
%%
