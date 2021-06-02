namespace Powder
{
	const char* grammarProductionsJson = "\
    {\r\n\
        \"statement-list\": [\r\n\
            [\"statement-list\", \";\", \"statement-list\"],\r\n\
            [\"statement\"]\r\n\
        ],\r\n\
        \"statement\": [\r\n\
            [\"if-statment\"],\r\n\
            [\"while-statement\"],\r\n\
            [\"do-while-statment\"],\r\n\
            [\"fork-statement\"],\r\n\
            [\"function-definition\"],\r\n\
            [\"expression\"]\r\n\
        ],\r\n\
        \"if-statement\": [\r\n\
            [\"if\", \"(\", \"expression\", \")\", \"embedded-statement\"],\r\n\
            [\"if\", \"(\", \"expression\", \")\", \"embedded-statement\", \"else\", \"embedded-statement\"]\r\n\
        ],\r\n\
        \"while-statement\": [\r\n\
            [\"while\", \"(\", \"expression\", \")\", \"embedded-statement\"]\r\n\
        ],\r\n\
        \"do-while-statement\": [\r\n\
            [\"do\", \"block\", \"while\", \"(\", \"expression\", \")\"]\r\n\
        ],\r\n\
        \"fork-statement\": [\r\n\
            [\"fork\", \"block\", \"else\", \"block\"]\r\n\
        ],\r\n\
        \"function-definition\": [\r\n\
            [\"func\", \"identifier\", \"(\", \"identifier-list\", \")\", \"block\"]\r\n\
        ],\r\n\
        \"identifier-list\": [\r\n\
            [\"identifier-list\", \",\", \"identifier-list\"],\r\n\
            [\"identifier\"]\r\n\
        ],\r\n\
        \"embedded-statement\": [\r\n\
            [\"expression\"],\r\n\
            [\"block\"]\r\n\
        ],\r\n\
        \"block\": [\r\n\
            [\"{\", \"}\"],\r\n\
            [\"{\", \"statement-list\", \"}\"]\r\n\
        ],\r\n\
        \"expression\": [\r\n\
            [\"(\", \"expression\", \")\"],\r\n\
            [\"operand\", \"=\", \"operand\"],\r\n\
            [\"operand\", \"+\", \"operand\"],\r\n\
            [\"operand\", \"-\", \"operand\"],\r\n\
            [\"operand\", \"*\", \"operand\"],\r\n\
            [\"operand\", \"/\", \"operand\"]\r\n\
        ],\
        \"operand\": [\r\n\
            [\"expression\"],\r\n\
            [\"function-call\"],\r\n\
            [\"identifier\"],\r\n\
            [\"string-literal\"],\r\n\
            [\"number-literal\"]\r\n\
        ],\r\n\
        \"function-call\": [\r\n\
            [\"identifier\", \"(\", \")\"],\r\n\
            [\"identifier\", \"(\", \"argument-list\", \")\"]\r\n\
        ],\r\n\
        \"argument-list\": [\r\n\
            [\"argument-list\", \",\", \"argument-list\"],\r\n\
            [\"argument\"]\r\n\
        ],\r\n\
        \"argument\": [\r\n\
            [\"expression\"]\r\n\
        ]\r\n\
    }\r\n\
	";
}