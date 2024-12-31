#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Tokens
typedef enum {
    TokenDigit, 
	TokenOperator,
    TokenMainMethod, 
	TokenLPar, 
	TokenRPar,
    TokenLeftB, 
	TokenRightB, 
	TokenPrint, 
	TokenEqual, 
	TokenTerminator,
    TokenIdentity, 
	TokenString, 
	TokenComma, 
	TokenInput
} TokenType;

// Struct of code
typedef struct {
    TokenType type;
    int value;         // For Numbers
    char name[256];     // For Identifiers (variable names)
    char strValue[256]; // For Strings
} Token;

// Variable structure for storing variables and their values
typedef struct {
    char name[256];
    int value; // For Integers
    char strValue[256]; // Add this for string variables
} Variable;

Variable variables[256];
int variableCount = 0;

// Declare as a global variable
int parseInput(Token *tokens, int *pos);

// Function to find or add a variable
int getOrAddVariable(const char *name) {
    int i;
    for (i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
    strcpy(variables[variableCount].name, name);
    variables[variableCount].value = 0;
    return variableCount++;
}

// Tokenize Functions
Token *tokenize(const char *input) {
    Token *tokens = malloc(256 * sizeof(Token));
    int pos = 0;

    while (*input) {
        if (isdigit(*input)) {
            tokens[pos].type = TokenDigit;
            tokens[pos].value = strtol(input, (char **)&input, 10);
            pos++;
        } else if (*input == '+') {
            tokens[pos].type = TokenOperator;
            tokens[pos++].value = '+';
            input++;
        } else if (*input == '-') {
            tokens[pos].type = TokenOperator;
            tokens[pos++].value = '-';
            input++;
        } else if (*input == '*') {
            tokens[pos].type = TokenOperator;
            tokens[pos++].value = '*';
            input++;
        } else if (*input == '/') {
            tokens[pos].type = TokenOperator;
            tokens[pos++].value = '/';
            input++;
        } else if (strncmp(input, "simula", 6) == 0) {
            tokens[pos++].type = TokenMainMethod;
            input += 6;
        } else if (strncmp(input, "=", 1) == 0) {
            tokens[pos++].type = TokenEqual;
            input += 1;
        } else if (strncmp(input, "ipakita", 7) == 0) {
            tokens[pos++].type = TokenPrint;
            input += 7;
        } else if (strncmp(input, "pasok", 5) == 0) {
            tokens[pos++].type = TokenInput;
            input += 5;
        } else if (*input == '(') {
            tokens[pos++].type = TokenLPar;
            input++;
        } else if (*input == ')') {
            tokens[pos++].type = TokenRPar;
            input++;
        } else if (*input == '{') {
            tokens[pos++].type = TokenLeftB;
            input++;
        } else if (*input == '}') {
            tokens[pos++].type = TokenRightB;
            input++;
        } else if (*input == ';') {
            tokens[pos++].type = TokenTerminator;
            input++;
        } else if (*input == ',') {
            tokens[pos++].type = TokenComma;
            input++;
        } else if (*input == '"') {
            tokens[pos].type = TokenString;
            input++; // Skip the opening quote
            int strPos = 0;
            while (*input != '"' && *input != '\0') {
                tokens[pos].strValue[strPos++] = *input++;
            }
            tokens[pos].strValue[strPos] = '\0';
            if (*input == '"') {
                input++; // Skip the closing quote
            }
            pos++;
        } else if (isalpha(*input)) {
            tokens[pos].type = TokenIdentity;
            int i = 0;
            while (isalpha(*input) || isdigit(*input) || *input == '_') {
                tokens[pos].name[i++] = *input++;
            }
            tokens[pos].name[i] = '\0';
            pos++;
        } else {
            input++; // Skip unrecognized characters (e.g., spaces)
        }
    }
    tokens[pos].type = TokenTerminator;
    return tokens;
}

// Function declarations
int parseFactor(Token *tokens, int *pos);
int parseTerm(Token *tokens, int *pos);
int parseExpression(Token *tokens, int *pos);

// Parsing function that handles parentheses
int parseFactor(Token *tokens, int *pos) {
    if (tokens[*pos].type == TokenLPar) {
        (*pos)++;
        int result = parseExpression(tokens, pos);
        if (tokens[*pos].type != TokenRPar) {
            printf("Error: Expected ')' after expression\n");
            exit(1);
        }
        (*pos)++;
        return result;
    } else if (tokens[*pos].type == TokenDigit) {
        int result = tokens[*pos].value;
        (*pos)++;
        return result;
    } else if (tokens[*pos].type == TokenIdentity) {
        int varIndex = getOrAddVariable(tokens[*pos].name);
        int result = variables[varIndex].value;
        (*pos)++;
        return result;
    } else {
        printf("Error: Expected a number, variable, or '('\n");
        exit(1);
    }
}

// Parsing function that handles multiplication and division
int parseTerm(Token *tokens, int *pos) {
    int result = parseFactor(tokens, pos);

    // Handle multiplication (*) and division (/)
    while (tokens[*pos].type == TokenOperator &&
           (tokens[*pos].value == '*' || tokens[*pos].value == '/')) {
        char op = tokens[*pos].value; // Store the operator
        (*pos)++;

        int nextValue = parseFactor(tokens, pos);

        if (op == '*') {
            result *= nextValue;
        } else if (op == '/') {
            if (nextValue == 0) {
                printf("Error: Division by zero\n");
                exit(1);
            }
            result /= nextValue;
        }
    }

    return result;
}

// Handle addition and subtraction
int parseExpression(Token *tokens, int *pos) {
    int result = parseTerm(tokens, pos);

    // Handle addition (+) and subtraction (-)
    while (tokens[*pos].type == TokenOperator &&
           (tokens[*pos].value == '+' || tokens[*pos].value == '-')) {
        char op = tokens[*pos].value; // Store the operator
        (*pos)++;

        int nextValue = parseTerm(tokens, pos);

        if (op == '+') {
            result += nextValue;
        } else if (op == '-') {
            result -= nextValue;
        }
    }

    return result;
}

// Function to parse the assignment statement
void parseAssignment(Token *tokens, int *pos) {
    if (tokens[*pos].type != TokenIdentity) {
        printf("Error: Expected a variable name before '='\n");
        exit(1);
    }

    // Retrieve variable name and its index
    char variableName[32];
    strcpy(variableName, tokens[*pos].name);
    int varIndex = getOrAddVariable(variableName);
    (*pos)++;

    if (tokens[*pos].type != TokenEqual) {
        printf("Error: Expected '=' after variable name\n");
        exit(1);
    }
    (*pos)++;

    // Handle assignment based on token type
    if (tokens[*pos].type == TokenInput) {
        // Parse Input statement
        int intValue = parseInput(tokens, pos);
        if (tokens[*pos - 1].strValue[0] != '\0') {
            strcpy(variables[varIndex].strValue, tokens[*pos - 1].strValue);
            variables[varIndex].value = 0; // Reset integer value
        } else {
            variables[varIndex].value = intValue;
            variables[varIndex].strValue[0] = '\0'; // Reset string value
        }
    } else if (tokens[*pos].type == TokenString) {
        // Assign string literals directly
        strcpy(variables[varIndex].strValue, tokens[*pos].strValue);
        variables[varIndex].value = 0; // Reset integer value
        (*pos)++;
    } else {
        // Parse general expressions
        int value = parseExpression(tokens, pos);
        variables[varIndex].value = value;
        variables[varIndex].strValue[0] = '\0'; // Reset string value
    }

    // Skip redundant semicolons
    while (tokens[*pos].type == TokenTerminator) {
        (*pos)++;
    }
}

// Printing using Print
void parsePrint(Token *tokens, int *pos) {
    if (tokens[*pos].type != TokenPrint) {
        printf("Error: Expected 'ipakita'\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != TokenLPar) {
        printf("Error: Expected '(' after 'ipakita'\n");
        exit(1);
    }
    (*pos)++;

    // Loop through the tokens inside ipakita(...)
    while (tokens[*pos].type != TokenRPar) {
        if (tokens[*pos].type == TokenString) {
            // Print a string literal
            printf("%s", tokens[*pos].strValue);
            (*pos)++;
        } else if (tokens[*pos].type == TokenIdentity) {
            // Handle variables (string or numeric)
            int varIndex = getOrAddVariable(tokens[*pos].name);
            if (strlen(variables[varIndex].strValue) > 0) {
                // Print string variable
                printf("%s", variables[varIndex].strValue);
            } else {
                // Print numeric variable
                printf("%d", variables[varIndex].value);
            }
            (*pos)++;
        } else if (tokens[*pos].type == TokenComma) {
            // Skip commas for separating ipakita arguments
            (*pos)++;
        } else {
            // Handle unexpected token
            printf("Error: Unexpected token in 'ipakita' statement\n");
            exit(1);
        }
    }

    if (tokens[*pos].type != TokenRPar) {
        printf("Error: Expected ')' after ipakita arguments\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != TokenTerminator) {
        printf("Error: Expected ';' after 'ipakita' statement\n");
        exit(1);
    }
    (*pos)++;

    printf("\n"); // Add a newline after the ipakita output
}

// Function to parse the Input statement
int parseInput(Token *tokens, int *pos) {
    if (tokens[*pos].type != TokenInput) {
        printf("Error: Expected 'pasok'\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != TokenLPar) {
        printf("Error: Expected '(' after 'pasok'\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != TokenString) {
        printf("Error: Expected a string prompt inside 'pasok'\n");
        exit(1);
    }
    char prompt[256];
    strcpy(prompt, tokens[*pos].strValue);
    printf("%s", prompt); // Print the prompt
    (*pos)++;

    if (tokens[*pos].type != TokenRPar) {
        printf("Error: Expected ')' after prompt\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != TokenTerminator) {
        printf("Error: Expected ';' after 'pasok' statement\n");
        exit(1);
    }
    (*pos)++;

    // Read user input
    char inputStr[256];
    if (!fgets(inputStr, sizeof(inputStr), stdin)) {
        printf("Error: Invalid input\n");
        exit(1);
    }

    // Remove newline character if present
    inputStr[strcspn(inputStr, "\n")] = '\0';

    // Attempt to parse the input as an integer
    char *endPtr;
    int inputValue = strtol(inputStr, &endPtr, 10);

    if (*endPtr == '\0') {
        // Entire input was successfully parsed as an integer
        return inputValue;
    } else {
        // Input is not a valid integer, treat it as a string
        strcpy(tokens[*pos - 1].strValue, inputStr);
        return 0; // Return 0 for strings as a placeholder
    }
}

void parseMainMethod(Token *tokens) {
    int pos = 0;

    if (tokens[pos].type != TokenMainMethod) {
        printf("Error: Missing 'simula'\n");
        return;
    }
    pos++;

    if (tokens[pos].type != TokenLPar || tokens[pos + 1].type != TokenRPar) {
        printf("Error: Missing '()'\n");
        return;
    }
    pos += 2;

    if (tokens[pos].type != TokenLeftB) {
        printf("Error: Missing '{' for body\n");
        return;
    }
    pos++;

    while (tokens[pos].type != TokenRightB) {
        if (tokens[pos].type == TokenIdentity && tokens[pos + 1].type == TokenEqual) {
            parseAssignment(tokens, &pos); // Handle assignment using '='
        } else if (tokens[pos].type == TokenPrint) {
            parsePrint(tokens, &pos);
        } else if (tokens[pos].type == TokenInput) {
            parseInput(tokens, &pos); // Handle Input statement
        } else {
            printf("Error: Unexpected token in body\n");
            exit(1);
        }
    }
    pos++;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    char input[256];
    size_t len = fread(input, 1, sizeof(input) - 1, file);
    input[len] = '\0';  // Null-terminate the string

    fclose(file);

    Token *tokens = tokenize(input);

    parseMainMethod(tokens);

    free(tokens);
    return 0;
}
