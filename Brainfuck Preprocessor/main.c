#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define ERROR_EXIT(reason) {fprintf(stderr, "ERROR: %s\n", reason); exit(EXIT_FAILURE);}

#define DEBUG(a) puts(a)
//#define DEGUB(a)

struct DynArrChar {
	size_t size, capacity;
	char * arr;
};

struct DynArrDynArrChar {
	size_t size, capacity;
	struct DynArrChar * arr;
};

struct DynArrChar * appendDynArrChar(struct DynArrChar * array, char value) {
	if (array->size >= array->capacity) {
		array->capacity *= 2;
		char * temp = realloc(array->arr, sizeof(char) * array->capacity);
		if (!temp) return NULL;
		array->arr = temp;
	}
	array->arr[array->size] = value;
	array->size++;
	return array;
}

struct DynArrDynArrChar * appendDynArrDynArrChar(struct DynArrDynArrChar * array, struct DynArrChar value) {
	if (array->size >= array->capacity) {
		array->capacity *= 2;
		struct DynArrChar * temp = realloc(array->arr, sizeof(struct DynArrChar) * array->capacity);
		if (!temp) return NULL;
		array->arr = temp;
	}
	array->arr[array->size] = value;
	array->size++;
	return array;
}


struct {
	struct DynArrDynArrChar names, bodies;
} functions;


bool isBrainfuck(int c) {
	switch (c) {
	case '[':
	case ']':
	case '>':
	case '<':
	case '+':
	case '-':
	case '.':
	case ',':
		return true;
	default:
		return false;
	}
}

bool parse_function(const FILE * file) {
	char c;

	int i = 0;
	while ((c = getc(file)) != EOF) {
		if (!"unction "[i]) break;
		if (c != "unction "[i]) return false;
		i++;
	}
	if (c == EOF) return false;

	DEBUG("function directive is good");

	i = 0;
	struct DynArrChar name = { .size = 0,.capacity = 10,.arr = malloc(sizeof(char) * 10) },
					  body = { .size = 0,.capacity = 10,.arr = malloc(sizeof(char) * 10) };
	do {
		if (isspace(c)) {
			if (i) break;
		}
		else {
			appendDynArrChar(&name, c);
			i++;
		}
	} while ((c = getc(file)) != EOF);
	if (c == EOF) return false;
	appendDynArrChar(&name, 0);

	DEBUG("Function Name:");
	DEBUG(name.arr);

	while (c != '\n' && (c = getc(file)) != EOF);
	if (c == EOF) return false;

	do {
		appendDynArrChar(&body, c);
	} while ((c = getc(file)) != EOF && c != '#');
	if (c == EOF) return false;
	appendDynArrChar(&body, 0);

	DEBUG("Function Body:");
	DEBUG(body.arr);

	i = 0;
	while ((c = getc(file)) != EOF) {
		if (!"end function "[i]) break;
		if (c != "end function "[i]) return false;
		i++;
	}
	if (c == EOF) return false;

	i = 0;
	do {
		if (!name.arr[i]) break;
		if (c != name.arr[i]) return false;
		i++;
	} while ((c = getc(file)) != EOF);
	if (c == EOF) return false;

	DEBUG("end function directive is good");

	appendDynArrDynArrChar(&functions.names, name);
	appendDynArrDynArrChar(&functions.bodies, body);

	return true;
}

bool parse_call(const FILE * file, struct DynArrChar * name) {

	char c;

	int i = 0;
	while ((c = getc(file)) != EOF) {
		if (!"all "[i]) break;
		if (c != "all "[i]) return false;
		i++;
	}
	if (c == EOF) return false;

	DEBUG("call directive is good");

	i = 0;
	do {
		if (isspace(c)) {
			if (i) break;
		}
		else {
			appendDynArrChar(name, c);
			i++;
		}
	} while ((c = getc(file)) != EOF);
	appendDynArrChar(name, 0);
	
	DEBUG("Call Name:");
	DEBUG(name->arr);

	return true;
}

bool paste_function(struct DynArrChar name, const FILE * file, bool minify) {
	for (size_t i = 0; i < functions.names.size; i++) {
		if (strcmp(functions.names.arr[i].arr, name.arr) == 0) {
			char * s = functions.bodies.arr[i].arr;
			DEBUG(s);
			for (char c = *s; *s; (c = *s, s++)) {
				if (isBrainfuck(c) || !minify) {
					putc(c, file);
				}
			}
			return true;
		}
	}
	return false;
}


int main(int argc, char** args) {

	char* inputFile = NULL;
	char* outputFile = "out.bf";
	char minify = 0;

	for (int i = 1; i < argc; i++) {
		if (args[i][0] == '-') {
			if (args[i][1] == 'o') {
				i++;
				if (i >= argc) ERROR_EXIT("Expected Output File Name");
				outputFile = args[i];
			}
			else if (args[i][1] == 'i') {
				i++;
				if (i >= argc) ERROR_EXIT("Expected Input File Name");
				inputFile = args[i];
			}
			else if (args[i][1] == 'm') {
				minify = 1;
			}
		}
	}

	if (!inputFile) ERROR_EXIT("No Input File Given");

	const FILE * inFile = fopen(inputFile, "r");
	if (!inFile) ERROR_EXIT("Could Not Open Input File");
	const FILE * outFile = fopen(outputFile, "w");
	if (!outFile) ERROR_EXIT("Could Not Open Output File");

	struct DynArrDynArrChar names = { .size = 0,.capacity = 10,.arr = malloc(sizeof(struct DynArrChar) * 10) },
							bodies = { .size = 0,.capacity = 10,.arr = malloc(sizeof(struct DynArrChar) * 10) };

	functions.names = names;
	functions.bodies = bodies;

	char c;
	bool directive = false;
	while ((c = getc(inFile)) != EOF) {
		if (directive) {
			if (c == '\n') {
				directive = false;
				DEBUG("end directive");
			}
			else if (c == 'f') {
				DEBUG("parse function");
				bool r = parse_function(inFile);
				if (!r) DEBUG("parse function failed");
			}
			else if (c == 'c') {
				struct DynArrChar name = { .size = 0,.capacity = 10,.arr = malloc(sizeof(char) * 10) };
				parse_call(inFile, &name);
				paste_function(name, outFile, minify);
			}
		}
		else if (c == '#') {
			directive = true;
			DEBUG("start directive");
		}
		else if (isBrainfuck(c) || !minify) {
			putc(c, outFile);
		}
	}

	puts("Input File:");
	printf("\t%s\n\n", inputFile);
	puts("Output File:");
	printf("\t%s\n\n", outputFile);

	puts("Functions:");
	for (size_t i = 0; i < functions.names.size; i++) {
		printf("%s\n", functions.names.arr[i].arr);
		printf("%s\n", functions.bodies.arr[i].arr);
	}

	return EXIT_SUCCESS;
}