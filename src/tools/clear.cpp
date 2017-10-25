#include <stdio.h>

// g++ ./tools/clear.cpp -o ./bin/clear

typedef void Parser(char, FILE *);
Parser* currentParser;

void InStringParser(char c, FILE *output_file);
void InCharParser(char c, FILE *output_file);
void AfterSlashParser(char c, FILE *output_file);
void InInlineCommentParser(char c, FILE *output_file);
void InMultilineCommentParser(char c, FILE *output_file);

void NormalParser(char c, FILE *output_file)
{
    switch (c)
    {
    case '\"':
        currentParser = InStringParser;
        break;
    case '\'':
        currentParser = InCharParser;
        break;
    case '/':
        currentParser = AfterSlashParser;
        // no need to print
        return;
    }
    fprintf(output_file, "%c", c);
}

bool inStringLastEscape = false;
void InStringParser(char c, FILE *output_file)
{
    fprintf(output_file, "%c", c);
    if (c == '\"' && !inStringLastEscape)
        currentParser = NormalParser; // inStringLastEscape will be false
    inStringLastEscape = (c == '\\') && !inStringLastEscape;
}

bool inCharLastEscape = false;
void InCharParser(char c, FILE *output_file)
{
    fprintf(output_file, "%c", c);
    if (c == '\'' && !inCharLastEscape)
        currentParser = NormalParser; // inCharLastEscape will be false
    inCharLastEscape = (c == '\\') && !inCharLastEscape;
}

void AfterSlashParser(char c, FILE *output_file)
{
    switch(c)
    {
    case '/':
        currentParser = InInlineCommentParser;
        return;
    case '*':
        currentParser = InMultilineCommentParser;
        return;
    default:
        fprintf(output_file, "/%c", c);
    }
}

bool inInlineLastEscape = false;
void InInlineCommentParser(char c, FILE *output_file)
{
    if (c == '\n' && !inInlineLastEscape)
    {
        fprintf(output_file, "\n");
        currentParser = NormalParser;
    }
    inInlineLastEscape = (c == '\\') && !inInlineLastEscape;
}

bool inMultilineLastStar = false;
void InMultilineCommentParser(char c, FILE *output_file)
{
    if (c == '/' && inMultilineLastStar)
    {
        fprintf(output_file, " ");
        currentParser = NormalParser; // inMultilineLastStar will be false
    }
    inMultilineLastStar = (c == '*');
}

int main(int argc, char* argv[])
{
    // открыть файлы и всё такое

    currentParser = NormalParser;

	FILE * input_file, * output_file;

	input_file = fopen("/Users/forman/Desktop/Work/proxy/src/aming1.cpp", "r");
	output_file = fopen("/Users/forman/Desktop/Work/proxy/src/aming3.cpp", "w");

	if(input_file == NULL) perror("Error opening file");
	else {
		char symbol;
		while ((symbol = fgetc(input_file)) != EOF)
			currentParser(symbol, output_file);

		fclose(input_file);
		fclose(output_file);
	}

    return 0;
}
