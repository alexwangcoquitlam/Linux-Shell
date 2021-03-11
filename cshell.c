#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct logEntry{
	char *name;
	struct tm * time;
	int code;
}logEntry;

typedef struct EnvVar{
	char *name;
	char *value
}EnvVar;

int envVarCount = 0;
int envVarLength = 100;

char *readline(){
	int lineSize = 100;
	int counter = 0;
	int active = 1;
	char *currLine = malloc(sizeof(char)*lineSize);	
	char c;
	
	while (active == 1){
		c = getchar();
		
		if (c == EOF || c == '\n'){
			currLine[counter] = '\0';
			return currLine;
		}
		else{
			currLine[counter] = c;
			counter++;
		}
		
		if(counter > lineSize){
			lineSize += 100;
			currLine = realloc(currLine, lineSize*sizeof(char));
		}
	}
}

char **SeparateTokens(char *line){
	int counter = 0;
	int lineLength = sizeof(line);
	char **tokens = malloc(lineLength*sizeof(char*));
	char *token;
	
	token = strtok(line, " \t\r\n");
	
	while (token != NULL){
		tokens[counter] = token;
		counter++;
		
		if(counter > lineLength){
			lineLength += 100;
			tokens = realloc(tokens, lineLength*sizeof(char*));
		}
		
		token = strtok(NULL, " \t\r\n");
	}
	
	tokens[counter] = NULL;
	return tokens;
}

int Execute(char **args, char *colour, struct logEntry *logEntries, int logCount, char *currentColour, struct EnvVar *EnvVars){
	int printCounter = 1;
	int found = 0;
	
	// Empty command was entered
	if (args[0] == NULL){return NULL;}
	
	
	// Checks if creating/modifying a variable
	else if(args[0][0] == '$' && args[1] && args[2])  {
		if (args[1][0] == '='){
			if (envVarCount > 0){
				for(int i = 0; i < envVarCount; i++){
					if (strcmp(args[0], EnvVars[i].name) == 0){
						printf("%sVariable already defined with value %s. Value has been changed to %s.\n", currentColour, EnvVars[i].value, args[2]);
						EnvVars[i].value = args[2];
						return 61;
					}
				}
				if (found == 0){
					CreateEnvVar(args, EnvVars);
					return 6;
				}
			}
			else{
				CreateEnvVar(args, EnvVars);
				return 6;
			}
		}
		else {
			printf("%sIncorrect format supplied for command $VAR = value.\n", currentColour);
			return NULL;
		}
	}
	
	// Stops the shell
	else if (strcmp(args[0], "exit") == 0){
		printf("%sClosing shell...\n", currentColour);
		exit(1);
		return 1;
	}
	
	// Command "theme", changes the colour of the shell, supports: red, green/default, yellow, white, and blue. The text input is always white.
	// Purple and cyan did work when I was first implementing this function, but after I added EnvVar support they broke and began printing the custome error message
	else if (strcmp(args[0], "theme") == 0 && args[1] != NULL){
		if (strcmp(args[1], "white") == 0){
			*colour = "\e[1;37m";
		}
		else if(strcmp(args[1], "default") == 0 || strcmp(args[1], "green") == 0){
			*colour = "\e[1;32m";	
		}
		else if(strcmp(args[1], "red") == 0){
			*colour = "\e[1;31m";}
		else if(strcmp(args[1], "blue") == 0){
			*colour = "\e[1;34m";}
		else if(strcmp(args[1], "yellow") == 0){
			*colour = "\e[1;33m";}
		/*else if(strcmp(args[1], "purple") == 0){
			*colour = "\e[1;35m";}
		else if(strcmp(args[1], "cyan") == 0){
			*colour = "\e[1;36m";}*/
		else{
			printf("%sColour not supported, the supported colours are: green, red, yellow, white, and blue.\n", currentColour);
		}
		return 2;			
	}
	if (strcmp(args[0], "theme") == 0)
		printf("%sPlease define a colour.\n", currentColour);
		
	// Print command that has a counter to check if the array has ended
	else if (strcmp(args[0], "print") == 0){
		while (args[printCounter] != NULL){
			if (args[printCounter][0] == '$'){
				for(int i = 0; i < envVarCount; i++){
					if (strcmp(args[printCounter], EnvVars[i].name) == 0){
						printf("%s%s ",currentColour, EnvVars[i].value);
						found = 1;
						break;
					}	
				}
				if (found != 1){
					printf("%sNo value found for %s. ",currentColour, args[printCounter]);
				}
			}
			else
				printf("%s%s ",currentColour, args[printCounter]);
			printCounter++;
		}
		printf("\n");
		return 3;
	}
	
	// log
	else if (strcmp(args[0], "log") == 0){
		if(logCount == 0){
			printf("%sYou have used no commands this session.\n", currentColour);
			return 4;
		}
		else{
			printf("%sYou have used %d commands this session: \n", currentColour, logCount);
			for(int i = 0; i < logCount; i++){
				printf("%s%s\t%s\t%d\n", currentColour, strtok(asctime(logEntries[i].time), "\n"), logEntries[i].name, logEntries[i].code);
			}
			return 4;
		}
	}
	
	// change directory
	else if (strcmp(args[0], "cd") == 0){
		if (args[1]){
			int returnCode = chdir(args[1]);
			if(returnCode != 0){
				printf("%sDirectory not found.\n", currentColour);
				return NULL;
			}
			else{
				return 8;
			}
		}
		else{
			printf("%sPlease specify a directory.\n", currentColour);
			return NULL;
		}
	}
	
	// Non-built in functions
	else{
		int p[2];
		pid_t pid;
		int *status;
		
		pid = fork();
		if(pipe(p) < 0){
			perror("cshell");
			return NULL;
		}
		
		if(pid == 0 && execvp(args[0],args) == -1){
			perror("cshell");
			exit(0);
			return NULL;
		}
		else if(pid < 0){
			printf("Error creating child process.\n");
			perror("cshell");
		}
		else{	
				
			//execvp(args[0], args);
			//exit(0);
			wait(NULL);
			return 7;
		}
	}

	return NULL;
}

void CreateEnvVar(char **args, struct EnvVar *envVars){
	envVars[envVarCount].name = strdup(args[0]);
	envVars[envVarCount].value = strdup(args[2]);
	envVarCount++;
	if (envVarCount+1 > envVarLength){
		envVarLength += 100;
		envVars = realloc(envVars, envVarLength*sizeof(EnvVar));
	}
}

int logCommand(char **args, struct logEntry *logEntries, int logEntriesCounter, int status){
	time_t timeInSeconds;
	time(&timeInSeconds);
	logEntries[logEntriesCounter].name = strdup(args[0]);
	logEntries[logEntriesCounter].time = malloc(sizeof(struct tm));
	memcpy(logEntries[logEntriesCounter].time, localtime(&timeInSeconds), sizeof(struct tm));
	logEntries[logEntriesCounter].code = status;
	logEntriesCounter++;
	return logEntriesCounter;
}

int main(int argc, char *argv[]){
	if (argc > 2){
		printf("Too many arguments supplied.\n");
		exit(0);
	}
	
	else if (argc == 1){
		printf("She sells cshells by the sea shore.\n");
		int active = 1;
		char *line;
		char **args;
		int logLength = 10;
		logEntry* logEntries = malloc(logLength*sizeof(logEntry));
		int logEntriesCounter = 0;
		int envVarLength = 100;
		EnvVar* envVars = malloc(envVarLength*sizeof(EnvVar));
		int status;
		char *colour = malloc(sizeof(char)*9);
		colour = "\e[1;32m";
		
		while(active == 1){
			printf("%scshell$ \e[0;37m", colour);
			line = readline();
			args = SeparateTokens(line);
			status = Execute(args, &colour, logEntries, logEntriesCounter, colour, envVars);
			if(status){
				logEntriesCounter = logCommand(args, logEntries, logEntriesCounter, status);
				if(logEntriesCounter+1 > logLength){
					logLength += 10;
					logEntries = realloc(logEntries, logLength*sizeof(logEntry));
				}
			}		
			free(line);
			free(args);
		}
	}
	else{
		FILE* testScript;
		char script[50] = "./";
		strcat(script, argv[1]);	
		testScript = fopen(script, "r");
		if (testScript == NULL){
			printf("Could not open file.\n");
			exit(0);
		}
		int active = 1;
		char *line = (char*)malloc(100);
		char **args;
		int logLength = 10;
		logEntry* logEntries = malloc(logLength*sizeof(logEntry));
		int logEntriesCounter = 0;
		EnvVar* envVars = malloc(envVarLength*sizeof(EnvVar));
		int status;
		char *colour = malloc(sizeof(char)*9);
		colour = "\e[1;32m";
		while(active == 1){
			if(!fgets(line, 100, testScript)){
				fclose(testScript);
				exit(0);
			}
			//printf("%scshell$ \e[0;37m", colour);
			//printf("%s", line);
			args = SeparateTokens(line);
			status = Execute(args, &colour, logEntries, logEntriesCounter, colour, envVars);
			if(status){
				logEntriesCounter = logCommand(args, logEntries, logEntriesCounter, status);
				if(logEntriesCounter+1 > logLength){
					logLength += 10;
					logEntries = realloc(logEntries, logLength*sizeof(logEntry));
				}
			}
			free(args);
			
			// Freeing the line gives me double free or corruption after exactly 9 iterations of the loop in script mode, not sure exactly why.
			//free(line);
			//printf("freed line\n");
		}
		fclose(testScript);
	}
	return 0;
}
