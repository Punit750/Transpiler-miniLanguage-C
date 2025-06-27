// CITS2002 Project 1 2024
// Student1: 23775211 Prem Patel
// Student2: 23905993 Punit Patil
// Platform apple macOS m1 chip

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_IDENTIFIER_LENGTH 12

void close_c_file(FILE  *c_file , FILE *function_file);

int is_real_constant(const char *str) {
    char *end;
    strtod(str, &end);
    return *end == '\0'; // true if the whole string is a valid number
}

// Function to check if a string is a valid lowercase identifier
int is_lowercase_identifier(const char *str) {
    int len = strlen(str);
    if (len < 1 || len > MAX_IDENTIFIER_LENGTH) return 0;
    for (int i = 0; i < len; i++) {
        if (!islower(str[i]) || !isalpha(str[i])) return 0;
    }
    return 1;
}

// Function to trim leading and trailing whitespace
char *syntax_space(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Null terminate
    *(end + 1) = '\0';

    return str;
}

int ends_with_ml(const char *path_file) {
    const char *dot = strrchr(path_file, '.');
    if (dot && strcmp(dot, ".ml") == 0) {
        return 1;
    }
    return 0;
}

int check_syntax(FILE *file) {
    char line[MAX_LINE_LENGTH];
    int expecting_indented_line = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character

        // Skip comments
        if (line[0] == '#') {
            continue;
        }

        // Work with a copy of the original line for parsing
        char line_copy[MAX_LINE_LENGTH];
        strcpy(line_copy, line);

        // Check for function definitions
        if (strstr(line_copy, "function") == line_copy) {
            char *function_def = line_copy + strlen("function ");
            char *function_name = strtok(function_def, " ");
            function_name = syntax_space(function_name);

            if (!is_lowercase_identifier(function_name)) {
                fprintf(stderr, "! Error: Function name '%s' must be a lowercase identifier.\n", function_name);
                return 1;
            }

            // Check the parameters
            char *parameter = strtok(NULL, " ");
            while (parameter) {
                parameter = syntax_space(parameter);
                if (!is_lowercase_identifier(parameter)) {
                    fprintf(stderr, "! Error: Parameter '%s' must be a lowercase identifier.\n", parameter);
                    return 1;
                }
                parameter = strtok(NULL, " ");
            }

            expecting_indented_line = 1;
        } else if (expecting_indented_line) {
            if (line[0] != '\t') {
                fprintf(stderr, "! Error: Line after function definition must be indented with a tab.\n");
                return 1;
            }

            // Work with a copy to validate identifiers
            strcpy(line_copy, line);
            char *token = strtok(line_copy, " \t<-()+*");
            int valid = 1;
            while (token != NULL) {
                if (!is_lowercase_identifier(token) && !is_real_constant(token)) {
                    fprintf(stderr, "! Error: Invalid identifier or constant '%s' in indented line.\n", token);
                    valid = 0;
                    break;
                }
                token = strtok(NULL, " \t<-()+*");
            }

            if (!valid) {
                return 1;
            }

            expecting_indented_line = 0;
        } else {
            // Check for assignment or other statements
            if (strchr(line, '<') && strchr(line, '-')) {
                strcpy(line_copy, line);
                char *variable = strtok(line_copy, "<-");
                char *value = strtok(NULL, "<-");
                variable = syntax_space(variable);
                value = syntax_space(value);

                if (!is_lowercase_identifier(variable)) {
                    fprintf(stderr, "! Error: Variable '%s' must be a lowercase identifier.\n", variable);
                    return 1;
                }

                if (!is_lowercase_identifier(value) && !is_real_constant(value)) {
                    fprintf(stderr, "! Error: Value '%s' must be a real constant.\n", value);
                    return 1;
                }

            } else if (strstr(line, "print") || strstr(line, "return")) {

            } else if (strchr(line, '(') && strchr(line, ')')) {

            } else {
                fprintf(stderr, "! Error: Invalid statement: %s\n", line);
                return 1;
            }
        }
    }

    return 0;
}



typedef struct {
    char name[50];
    char type[10];
} Variable;

Variable variables[100];
int variable_count = 0;

char functionNames[100][100];  // Array to store function names
int functionCount = 0; 

void add_variable(const char *name , const char *type){
    strcpy(variables[variable_count].name , name);
    strcpy(variables[variable_count].type , type);
    variable_count ++;

}

int is_double(const char *str){
    return strchr(str , '.') != NULL;
} 

void variable_name_value(const char *s, FILE *c_file, FILE *header_file) {
    char var_name[50];
    char value_str[50];
    char functionName[50];

    // Parse the variable assignment line (e.g., "x <- 5" or "x <- us(12, 6)")
    if (sscanf(s, "%s <- %[^\n]", var_name, value_str) == 2) {

        // Extract the potential function name (e.g., "us" from "us(12, 6)")
        sscanf(value_str, "%s(", functionName);

        // Check if functionName is in the stored function list
        int isFunctionCall = 0;
        for (int i = 0; i < functionCount; i++) {
            if (strcmp(functionNames[i], functionName) == 0) {
                isFunctionCall = 1;
                break;
            }
        }
        // If it's a function call, store the variable with type double
        if (isFunctionCall) {
            // Store the variable name and type double
            add_variable(var_name, "double");
            return;
        }
        // Check if the value is a double
        if (is_double(value_str)) {
            double value = atof(value_str);
            // Declare the variable in the C file
            fprintf(c_file, "double %s = %f;\n", var_name, value);
            // Add the extern declaration in the header file
            fprintf(header_file, "extern double %s;\n", var_name);
            // Add variable to the variable tracking system
            add_variable(var_name, "double");
        }
        // Check if the value is an integer
        else {
            int value = atoi(value_str);
            // Declare the variable in the C file
            fprintf(c_file, "int %s = %d;\n", var_name, value);
            // Add the extern declaration in the header file
            fprintf(header_file, "extern int %s;\n", var_name);
            // Add variable to the variable tracking system
            add_variable(var_name, "int");
        }
    }
}

int is_number(const char *str){
    char *endptr;
    strtod(str , &endptr);
    return *endptr == '\0' ;
}

void transpile_print_statement(const char *line, FILE *c_file) {
    char function_call[100];

    // First, check if the line starts with a function call (without 'print')
    sscanf(line, "%s", function_call);
    
    for (int i = 0; i < functionCount; i++) {
        if (strncmp(functionNames[i], function_call, strlen(functionNames[i])) == 0) {
            return;  // Handle function call without 'print' and return without processing
        }
    }
    // Now handle the normal print statement processing
    char print_expr[100];
    // Parse the print statement to capture the expression
    if (sscanf(line, "print %[^\n]", print_expr) == 1) {
        // Check if the expression contains a function call
        char function_name[100];
        if (sscanf(print_expr, "%[^ (]", function_name) == 1) {
            for (int i = 0; i < functionCount; i++) {
                if (strcmp(functionNames[i], function_name) == 0) {
                    // We have a function call inside the print statement, process it
                    char args[100];
                    sscanf(print_expr + strlen(function_name), "(%[^)])", args);
                    return;
                }
            }
        }
        // If it's not a function call, handle the rest as usual (i.e., variables and numbers)
        char modified_expr[200] = "";
        char *ptr = print_expr;
        int is_expression_double = 0;

        // Loop through the expression character by character
        while (*ptr != '\0') {
            // If the current character is a space or operator, add it directly to the output
            if (*ptr == ' ' || *ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/') {
                strncat(modified_expr, ptr, 1);
            } else {
                // Extract a potential variable or number
                char token[100] = "";
                int index = 0;
                while (*ptr != ' ' && *ptr != '\0' && *ptr != '+' && *ptr != '-' && *ptr != '*' && *ptr != '/') {
                    token[index++] = *ptr++;
                }
                token[index] = '\0';  // Null-terminate the token

                // Check if the token is a number
                if (is_number(token)) {
                    strcat(modified_expr, token);
                    // Check if it's a floating-point number
                    if (strchr(token, '.') != NULL) {
                        is_expression_double = 1;
                    }
                } else {
                    // Check if the token is a declared variable
                    int found_var = 0;
                    char var_type[10];

                    for (int i = 0; i < variable_count; i++) {
                        if (strcmp(variables[i].name, token) == 0) {
                            strcat(modified_expr, token);
                            strcpy(var_type, variables[i].type);
                            found_var = 1;
                            if (strcmp(var_type, "double") == 0) {
                                is_expression_double = 1;
                            }
                            break;
                        }
                    }

                    // If the variable was not found, replace it with 0 or 0.0
                    if (!found_var) {
                        strcat(modified_expr, is_expression_double ? "0.0" : "0");
                    }
                }
                continue;  // Skip the rest of the while to recheck operators
            }
            ptr++;  // Move to the next character
        }

        // Print the final constructed expression, based on whether it's a double or int
        if (is_expression_double) {
            fprintf(c_file, "printf(\"%%f\\n\", (double)(%s));\n", modified_expr);
        } else {
            fprintf(c_file, "printf(\"%%d\\n\", (int)(%s));\n", modified_expr);
        }
    }
}

int isIdented(const char *s){
    return isspace(s[0]);
}

void handle_local_function_assignment(const char *s, FILE *c_file) {
    char var_name[50];
    char functionCall[100];
    char functionName[50];

    // Parse the variable name and function call (e.g., "x <- us(12, 6)")
    if (sscanf(s, "%s <- %[^\n]", var_name, functionCall) == 2) {
        
        // Extract the function name from the function call
        sscanf(functionCall, "%s(", functionName);

        // Check if the function name is in the stored function list
        int isFunctionValid = 0;
        for (int i = 0; i < functionCount; i++) {
            if (strcmp(functionNames[i], functionName) == 0) {
                isFunctionValid = 1;
                break;
            }
        }
        // Generate the initialization code only if the function name is valid
        if (isFunctionValid) {
            fprintf(c_file, "double %s = %s;\n", var_name, functionCall);
        } else {
            return;
        }
    }
}

void transpile_function(const char *s, FILE *function_file, FILE *header_file) {
    char functionName[100];
    char variables[256] = ""; // Initialize variables with an empty string
    char formattedVariables[256] = "";

    // Parse the function name and its variables (if any)
    if (sscanf(s, "function %s %[^\n]", functionName, variables) == 2 || sscanf(s, "function %s", functionName) == 1) {
        // Add function name to the global list
        strcpy(functionNames[functionCount], functionName);
        functionCount++;

        // If there are variables, process them
        if (strlen(variables) > 0) {
            char *token = strtok(variables, " ");
            while (token != NULL) {
                strcat(formattedVariables, "double ");
                strcat(formattedVariables, token);
                token = strtok(NULL, " ");
                if (token != NULL) {
                    strcat(formattedVariables, ", ");
                }
            }
        }

        // If no variables, formattedVariables will remain an empty string
        fprintf(function_file, "double %s(%s){\n", functionName, formattedVariables);
        fprintf(header_file, "double %s(%s);\n", functionName, formattedVariables);
    } else {
        return;
    }
}

void trim_whitespace(char *str) {
    // Trim leading spaces
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing spaces
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write the null terminator
    *(end + 1) = '\0';
}

void transpile_function_call(const char *s, FILE *c_file, FILE *function_file) {
    char buffer[256];
    char expression[256] = "";
    const char *ptr = s;
    char functionName[100];   // To store the function name
    char arguments[256] = ""; // Initialize arguments as an empty string to handle zero arguments
    int is_valid_function = 0;
    int functionFound = 0;    // To track if a function is found within an expression

    // Step 1: Handle print statements with complex expressions
    if (strncmp(s, "print ", 6) == 0) {
        ptr += 6;  // Move pointer past "print "

        // Process the entire expression, allowing for multiple function calls and operators
        while (*ptr != '\0') {
            // Skip any leading spaces or newlines
            while (*ptr == ' ' || *ptr == '\n') {
                ptr++;
            }
            // If an operator is encountered, append it directly to the expression
            if (*ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/') {
                strncat(expression, ptr, 1);  // Append operator to the expression
                ptr++;
                continue;  // Skip to the next character
            }
            // Parse the function name and arguments
            if (sscanf(ptr, "%[^()](%[^)])", functionName, arguments) == 2 || sscanf(ptr, "%[^()]( )", functionName) == 1) {
                trim_whitespace(functionName);
                trim_whitespace(arguments);

                // **Check for an empty function name** after trimming
                if (strlen(functionName) == 0) {
                    return; // Exit early if the function name is invalid
                }

                // Validate the function name only once
                is_valid_function = 0;
                for (int i = 0; i < functionCount; i++) {
                    if (strcmp(functionName, functionNames[i]) == 0) {
                        is_valid_function = 1;
                        
                        break;
                    }
                }

                if (is_valid_function) {
                    // Append the valid function call to the expression
                    if (strlen(arguments) > 0) {
                        snprintf(buffer, sizeof(buffer), "%s(%s)", functionName, arguments);
                    } else {
                        snprintf(buffer, sizeof(buffer), "%s()", functionName); // Handle no arguments case
                    }
                    // Check for buffer overflow before concatenating
                    if (strlen(expression) + strlen(buffer) < sizeof(expression)) {
                        strcat(expression, buffer);
                    } else {
                        printf("Error: Expression too long.\n");
                        return;
                    }
                    functionFound = 1;  // Mark that a function was found

                    // Move the pointer past the function call
                    const char *end_ptr = strchr(ptr, ')');
                    if (end_ptr != NULL) {
                        ptr = end_ptr + 1;
                    } else {
                        return;
                    }
                } else {
                    return; // Exit early since an invalid function was found
                }
            } else {
                // Move the pointer forward for non-function parts (like variables or operators)
                strncat(expression, ptr, 1);
                ptr++;
            }
        }
        // If a valid expression was built, transpile it
        if (functionFound) {
            // Transpile the entire expression
            fprintf(c_file, "double result = %s;\n", expression);
            fprintf(c_file, "int_double_checker(result);\n");
        }
    }
    // Step 2: Handle regular function calls (no print)
    else if (sscanf(s, "%[^()](%[^)])", functionName, arguments) == 2 || sscanf(s, "%[^()]( )", functionName) == 1) {
        trim_whitespace(functionName);
        trim_whitespace(arguments);

        // **Check for an empty function name** after trimming
        if (strlen(functionName) == 0) {
            return; // Exit early if the function name is invalid
        }

        // Validate the function name only once
        is_valid_function = 0;
        for (int i = 0; i < functionCount; i++) {
            if (strcmp(functionName, functionNames[i]) == 0) {
                is_valid_function = 1;
                break;
            }
        }

        if (is_valid_function) {
            // Handle function calls directly in the main function
            if (strlen(arguments) > 0) {
                fprintf(c_file, "%s(%s);\n", functionName, arguments);
            } else {
                fprintf(c_file, "%s();\n", functionName); // Handle no arguments case
            }
        } else {
            return;
        }
    }
}

void transpile_function_body(const char *s, FILE *function_file) {
    char variable1[100];
    char variable2[100];

    // Check if the line is indented (i.e., inside the function body)
    if (isIdented(s)) {

        // Handle print statement with a complex expression (e.g., print (a + b) * (c + d))
        if (sscanf(s, "    print %[^\n]", variable1) == 1) {
    // Assuming variable1 is a string representation of the expression or variable name
            fprintf(function_file, "double result = %s;\n", variable1);  // Store the result in a double variable
            fprintf(function_file, "if (result == (int)result) {\n");
            fprintf(function_file, "    printf(\"%%d\\n\", (int)result);\n");  // Print as int if there's no decimal part
            fprintf(function_file, "} else {\n");
            fprintf(function_file, "    printf(\"%%.6f\\n\", result);\n");  // Print as double otherwise
            fprintf(function_file, "}\n");
            fprintf(function_file, "return 0;\n");
            fprintf(function_file, "}\n");
        }
        else if (sscanf(s, "    print %s", variable1) == 1) {
            fprintf(function_file, "double result = %s;\n", variable1);
            fprintf(function_file, "if (result == (int)result) {\n");
            fprintf(function_file, "    printf(\"%%d\\n\", (int)result);\n");
            fprintf(function_file, "} else {\n");
            fprintf(function_file, "    printf(\"%%.6f\\n\", result);\n");
            fprintf(function_file, "}\n");
            fprintf(function_file, "return 0;\n");
            fprintf(function_file, "}\n");
        }

        // Handle return statement with a complex expression (e.g., return (a + b) * (c + d))
        else if (sscanf(s, " return %[^\n]", variable1) == 1) {
            fprintf(function_file, "double result = %s;\n", variable1);  // Store the result in a double variable
            fprintf(function_file, "if (result == (int)result) {\n");
            fprintf(function_file, "    return (int)result;\n");  // Return as int if there's no decimal part
            fprintf(function_file, "} else {\n");
            fprintf(function_file, "    return result;\n");  // Return as double otherwise
            fprintf(function_file, "}\n");
            fprintf(function_file, "}\n");
        }
// Handle return statement with a simple variable (e.g., return x)
        else if (sscanf(s, " return %s", variable1) == 1) {
            fprintf(function_file, "double result = %s;\n", variable1);
            fprintf(function_file, "if (result == (int)result) {\n");
            fprintf(function_file, "    return (int)result;\n");
            fprintf(function_file, "} else {\n");
            fprintf(function_file, "    return result;\n");
            fprintf(function_file, "}\n");
            fprintf(function_file, "}\n");
        }

        // Handle assignment with a complex expression (e.g., x <- (a + b) * (c + d))
        else if (sscanf(s, " %s <- %[^\n]", variable1, variable2) == 2) {
            fprintf(function_file, "double %s = (double)(%s);\n", variable1, variable2);
        }
        // Handle simple assignment (e.g., x <- y)
        else if (sscanf(s, " %s <- %s", variable1, variable2) == 2) {
            fprintf(function_file, "double %s = (double) %s;\n", variable1, variable2);
        }
    }
}

void close_c_file(FILE  *c_file , FILE *function_file){
    fprintf(c_file , "}\n");
    fclose(c_file);   
    fclose(function_file);
}

int opening_file(const char *path_file) {
    FILE *fp;
    char s[256];

    // Open the output C files
    FILE *c_file = fopen("translated_code.c", "w");
    if (c_file == NULL) {
        printf("Error opening the file\n");
        exit(EXIT_FAILURE);
    }

    FILE *function_file = fopen("translated_code1.c", "w");
    FILE *header_file = fopen("functions.h", "w");
    if (function_file == NULL) {
        printf("Error opening the file \n");
        exit(EXIT_FAILURE);
    }
    // Write the int_double_checker function into the C file
    // Write necessary headers
    fprintf(header_file, "#ifndef FUNCTIONS_H\n");
    fprintf(header_file, "#define FUNCTIONS_H\n\n");

    fprintf(function_file , "#include <stdio.h>\n");
    fprintf(function_file , "#include <stdlib.h>\n");
    fprintf(function_file , "#include \"functions.h\"\n\n");
    
    fprintf(c_file , "#include <stdio.h>\n");
    fprintf(c_file , "#include <stdlib.h>\n");
    fprintf(c_file , "#include \"functions.h\"\n\n");

    fprintf(c_file, "\n");
    fprintf(c_file, "void int_double_checker(double result) {\n");
    fprintf(c_file, "    if (result == (int)result) {\n");
    fprintf(c_file, "        printf(\"%%d\\n\", (int)result);\n");
    fprintf(c_file, "    } else {\n");
    fprintf(c_file, "        printf(\"%%.6f\\n\", result);\n");
    fprintf(c_file, "    }\n");
    fprintf(c_file, "}\n");


    // Open the input file
    fp = fopen(path_file, "r");
    if (fp == NULL) {
        printf("Error: cannot open file '%s'\n", path_file);
        return 1;
    }

    // Step 1: Process ONLY global variables and function headers in the first loop
    while (fgets(s, sizeof(s), fp) != NULL) {
        // Process global variables and function headers here
        //check_syntax(s);
        variable_name_value(s, c_file , header_file);   
        transpile_function(s, function_file, header_file);
        transpile_function_body(s , function_file);  
    }

    // Step 2: Write the start of the main function after processing global variables
    fprintf(c_file, "int main() {\n");

    // Step 3: Rewind the input file and process print statements and function calls in main
    rewind(fp);  // We need to reprocess the file now for main-specific code
    while (fgets(s, sizeof(s), fp) != NULL) {    
        handle_local_function_assignment(s,c_file);
        if (strstr(s, "(") && strstr(s, ")")) {
        transpile_function_call(s, c_file, function_file); // Handle function calls
        }   
        if (!isIdented(s)){
            transpile_print_statement(s, c_file);
        }
    }

    // Close input file
    fclose(fp);

    // Close C file properly
    close_c_file(c_file , function_file);

    // Finalize the header file
    fprintf(header_file, "\n#endif // FUNCTIONS_H\n");
    fclose(header_file);
    return 0;
}

void Executor(){
    int compile = system("gcc -o final translated_code.c translated_code1.c");
    if (compile != 0) {
        printf("Compilation failed with code: %d\n", compile);
    }

    int run = system("./final");
    if (run != 0) {
        printf("Execution failed with code: %d\n", run);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (ends_with_ml(argv[1])) {
        ///printf("The file ends with .ml\n");
        if (opening_file(argv[1]) != 0) {
            return EXIT_FAILURE;
        }   
        
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening file for syntax check");
            return EXIT_FAILURE;
        }
        
        if (check_syntax(file) != 0) {
            fclose(file);
            perror("Error in syntax of the file");
            return EXIT_FAILURE; // Exit if there's a syntax error
            fclose(file);
        }
        fclose(file);  

        Executor();    

        remove("translated_code.c");
        remove("translated_code1.c");
        remove("functions.h");
    } else {
        printf("The file does not end with .ml\n");
    }
    return EXIT_SUCCESS;
}
