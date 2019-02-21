#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum block{NOT, AND, OR, NAND, NOR, XOR, DEC, MUX};

typedef struct Node{
	char* name;
	int value;
	int skip;
	struct Node *next;
} node;

typedef struct Gate{
	int type;
	node * inputs;
	node * outputs;
	node * selectorArray;
	int numInput;
	int numOutput;
	int numSelector;
	struct Gate* next;
	int skipFlag;
} gate;

void reset(node* array, int length);
void addNode(node* head, char* target);
void addOperation(char* line);
gate* createGate(int gateOperation, int numInput, int numOutput, int numSelector);
void fillInput(int num, int numInput);
void simulate();
void resetTemp();
void checkInputs(gate* currentGate);
void fillOutput(char* target, int value);
void loadGateIO(gate* newGate, char* line, int numInput, int numOutput);
int findValue(char* target, node* head);

gate* operations;
node* tempVars;
node *inputVars;
node *outputVars;

int main(int argc, char** argv){
	FILE *fp;
	char buffer[500];
	char line[500];
	char *token;
	int numInput, numOutput, i, gray, j;
	//int skip = 0;
	operations = NULL;
	tempVars = NULL;

	//Check input arguments
	if (argc<2){
		printf("Invalid input argument(s).\n");
		return 0;
	}
	fp = fopen(argv[1],"r");
	if (fp == NULL){
		printf("Invalid circuit description file.\n");
		return 0;
	}

	//read file
	while(fgets(buffer, 500, fp) != NULL){
		strcpy(line, buffer);
		token = strtok(buffer, " ");
		if (strcmp(token, "INPUTVAR") == 0){
			token = strtok(NULL, " ");
			numInput = atoi(token);
			inputVars = (node*) malloc(sizeof(node) * numInput);
			for (i=0; i<numInput; i++){
				token = strtok(NULL, " \n");
				inputVars[i].name = (char*) malloc(100);
				memset(inputVars[i].name, '\0', 100);
				strcpy(inputVars[i].name,token);
				if (i == numInput-1){
					inputVars[i].next = NULL;
				} else {
					inputVars[i].next = &inputVars[i+1];
				}
			}
			reset(inputVars, numInput);

		} else if (strcmp(token, "OUTPUTVAR") == 0){
			token = strtok(NULL, " ");
			numOutput = atoi(token);
			outputVars = (node*) malloc(sizeof(node) * numOutput);
			for (i=0; i<numOutput; i++){
				token = strtok(NULL, " \n");
				outputVars[i].name = (char*) malloc(100);
				memset(outputVars[i].name, '\0', 100);
				strcpy(outputVars[i].name,token);
				if (i == numOutput-1){
					outputVars[i].next = NULL;
				} else {
					outputVars[i].next = &outputVars[i+1];
				}
			}
			reset(outputVars, numOutput);

		} else {
			addOperation(line);

		}
	}
	for (i=0; i<pow(2,numInput); i++){
		reset(inputVars, numInput);
		reset(outputVars, numOutput);
		resetTemp();
		gray = i ^ (i>>1); //gray code equivalent
		fillInput(gray, numInput); //load input with gray code
		simulate();
		for (j=0; j<numInput; j++){
			printf("%d ",inputVars[j].value);
		}
		for (j=0; j<numOutput; j++){
			printf("%d ", outputVars[j].value);
		}
		printf("\n");
	}
	return 0;
}

void simulate(){
	int skip, result, i, recv;
	do {
		gate* currentGate = operations;
		skip = 0;
		while (currentGate != NULL){
			switch(currentGate->type) {
				case NOT:
					checkInputs(currentGate);
					if (currentGate->skipFlag > 0){
						skip++;
					} else {
						result = !currentGate->inputs[0].value;
						currentGate->outputs[0].value = result;
						fillOutput(currentGate->outputs[0].name, result);
					}
					break;
				case AND:
					checkInputs(currentGate);
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						result = currentGate->inputs[0].value && currentGate->inputs[1].value;
						currentGate->outputs[0].value = result;
						fillOutput(currentGate->outputs[0].name, result);
					}
					break;
				case OR:
					checkInputs(currentGate);
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						result = currentGate->inputs[0].value || currentGate->inputs[1].value;
						currentGate->outputs[0].value = result;
						fillOutput(currentGate->outputs[0].name, result);
					}
					break;
				case NAND:
					checkInputs(currentGate);
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						result = currentGate->inputs[0].value && currentGate->inputs[1].value;
						currentGate->outputs[0].value = !result;
						fillOutput(currentGate->outputs[0].name, !result);
					}
					break;
				case NOR:
					checkInputs(currentGate);
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						result = currentGate->inputs[0].value || currentGate->inputs[1].value;
						currentGate->outputs[0].value = !result;
						fillOutput(currentGate->outputs[0].name, !result);
					}
					break;
				case XOR:
					checkInputs(currentGate);
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						result = currentGate->inputs[0].value ^ currentGate->inputs[1].value;
						currentGate->outputs[0].value = result;
						fillOutput(currentGate->outputs[0].name, result);
					}
					break;
				case DEC:
					checkInputs(currentGate);
					result = 0;
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						char code[50];
						memset(code, '\0', 50);
						if (currentGate->inputs[0].value == 1){
							code[0] = '1';
						} else {
							code[0] = '0';
						}
						for(i=1;i<currentGate->numInput;i++){
							if (currentGate->inputs[i].value == 1){
								if (code[i-1] == '0'){
									code[i] = '1';
								} else{
									code[i] = '0';
								}
							} else{
								code[i] = code[i-1];
							}
						}
						result = 0;
						for (i=0;i<strlen(code); i++){
							result += (int)((code[i]-'0')*pow(2,strlen(code)-1-i));
						}
						for (i=0;i<currentGate->numOutput;i++){
							currentGate->outputs[i].value = 0;
							fillOutput(currentGate->outputs[i].name, 0);
							if (result == i){
								currentGate->outputs[i].value = 1;
								fillOutput(currentGate->outputs[i].name, 1);
							}

						}
					}
					break;
				case MUX:
					checkInputs(currentGate);
					for (i=0;i<currentGate->numSelector;i++){
						if (strcmp(currentGate->selectorArray[i].name, "0") == 0){
							currentGate->selectorArray[i].value = 0;
							continue;
						} else if (strcmp(currentGate->selectorArray[i].name, "1") == 0){
							currentGate->selectorArray[i].value = 1;
							continue;
						} else if ((currentGate->selectorArray[i].name[0] >= 'a')
								&& (currentGate->selectorArray[i].name[0] <= 'z')) {
							recv = findValue(currentGate->selectorArray[i].name, tempVars);
							if (recv == -1){
								currentGate->selectorArray[i].skip = 1;
								currentGate->skipFlag++;

							} else {
								currentGate->selectorArray[i].value = recv;
								if (currentGate->selectorArray[i].skip == 1){
									currentGate->selectorArray[i].skip = 0;
									currentGate->skipFlag--;
								}
							}
						} else {
							recv = findValue(currentGate->selectorArray[i].name, inputVars);
							currentGate->selectorArray[i].value = recv;
						}
					}
					if (currentGate->skipFlag > 0){
						skip++;
						break;
					} else {
						char code[50];
						memset(code, '\0', 50);
						if (currentGate->selectorArray[0].value == 1) {
							code[0] = '1';
						} else {
							code[0] = '0';
						}
						for(i=1;i<currentGate->numSelector;i++){
							if (currentGate->selectorArray[i].value == 1){
								if (code[i-1] == '0'){
									code[i] = '1';
								} else{
									code[i] = '0';
								}
							} else{
								code[i] = code[i-1];
							}
						}
						result = 0;
						for (i=0;i<strlen(code); i++){
							result += (int)((code[i]-'0')*pow(2,strlen(code)-1-i));
						}
						currentGate->outputs[0].value = currentGate->inputs[result].value;
						fillOutput(currentGate->outputs[0].name, currentGate->inputs[result].value);
					}
			}
			currentGate = currentGate->next;
		}
	} while(skip != 0);
}

void checkInputs(gate* currentGate){
	int i, recv;
	for (i=0;i<currentGate->numInput;i++){
		if (strcmp(currentGate->inputs[i].name, "0") == 0){
			currentGate->inputs[i].value = 0;
			continue;
		} else if (strcmp(currentGate->inputs[i].name, "1") == 0){
			currentGate->inputs[i].value = 1;
			continue;
		} else if ((currentGate->inputs[i].name[0] >= 'a')
				&& (currentGate->inputs[i].name[0] <= 'z')) {
			recv = findValue(currentGate->inputs[i].name, tempVars);
			if (recv == -1){
				currentGate->inputs[i].skip = 1;
				currentGate->skipFlag++;

			} else {
				currentGate->inputs[i].value = recv;
				if (currentGate->inputs[i].skip == 1){
					currentGate->inputs[i].skip = 0;
					currentGate->skipFlag--;
				}
			}
		} else {
			recv = findValue(currentGate->inputs[i].name, inputVars);
			currentGate->inputs[i].value = recv;
		}
	}
}

int findValue(char* target, node* head){
	node* ptr = head;
	while (ptr != NULL){
		if (strcmp(ptr->name,target) == 0){
			return ptr->value;
		}
		ptr = ptr->next;
	}
	return -1;
}

void fillInput(int num, int numInput){
	int i = numInput-1;
	while(num>0){
		inputVars[i--].value = num % 2;
		num = num / 2;
	}
	while (i>=0){
		inputVars[i--].value = 0;
	}
	return;
}

void fillOutput(char* target, int value){
	node* ptr = outputVars;
	while (ptr != NULL){
		if (strcmp(ptr->name,target) == 0){
			ptr->value = value;
			break;
		}
		ptr = ptr->next;
	}
	ptr = tempVars;
	while (ptr != NULL){
		if (strcmp(ptr->name,target) == 0){
			ptr->value = value;
			break;
		}
		ptr = ptr->next;
	}

	return;
}

//Sets all value of array to -1
void reset(node* array, int length){
	int i;
	for (i=0; i<length; i++){
		array[i].value = -1;
	}
	return;
}

void resetTemp(){
	node* ptr = tempVars;
	while(ptr != NULL){
		ptr->value = -1;
		ptr = ptr->next;
	}
	return;
}


void addOperation(char* line){
	char original[500];
	strcpy(original,line);
	char* token = strtok(line, " ");
	int numInput, numOutput, numSelector, i;
	gate* newGate;
	if (strcmp(token,"NOT") == 0) {
		newGate = createGate(NOT, 1, 1, 0);
		loadGateIO(newGate, original, 1, 1);
		newGate->skipFlag = 0;

	} else if(strcmp(token,"AND") == 0) {
		newGate = createGate(AND, 2, 1, 0);
		loadGateIO(newGate, original, 2, 1);
		newGate->skipFlag = 0;

	} else if(strcmp(token,"OR") == 0){
		newGate = createGate(OR, 2, 1, 0);
		loadGateIO(newGate, original, 2, 1);
		newGate->skipFlag = 0;

	} else if(strcmp(token, "NAND") == 0) {
		newGate = createGate(NAND, 2, 1, 0);
		loadGateIO(newGate, original, 2, 1);
		newGate->skipFlag = 0;

	} else if(strcmp(token,"NOR") == 0) {
		newGate = createGate(NOR, 2, 1, 0);
		loadGateIO(newGate, original, 2, 1);
		newGate->skipFlag = 0;

	} else if(strcmp(token, "XOR") == 0) {
		newGate = createGate(XOR, 2, 1, 0);
		loadGateIO(newGate, original, 2, 1);
		newGate->skipFlag = 0;

	} else if(strcmp(token, "DECODER") == 0) {
		token = strtok(NULL, " ");
		numInput = atoi(token);
		numOutput = (int) pow(2,numInput);
		newGate = createGate(DEC, numInput, numOutput, 0);
		newGate->skipFlag = 0;
		for (i=0;i<numInput;i++){
			token = strtok(NULL, " ");
			if (token[0] == '1'){
				newGate->inputs[i].name = "1";
				newGate->inputs[i].value = 1;
			} else if (token[0] == '0'){
				newGate->inputs[i].name = "0";
				newGate->inputs[i].value = 0;
			} else {
				newGate->inputs[i].name = (char*) malloc(100);
				memset(newGate->inputs[i].name, '\0', 100);
				strcpy(newGate->inputs[i].name, token);
				newGate->inputs[i].value = -1;
			}
		}
		for (i=0;i<numOutput;i++){
			token = strtok(NULL, " \n");
			if (token[0] == '1'){
				newGate->outputs[i].name = "1";
				newGate->outputs[i].value = 1;
			} else if (token[0] == '0'){
				newGate->outputs[i].name = "0";
				newGate->outputs[i].value = 0;
			} else {
				newGate->outputs[i].name = (char*) malloc(100);
				memset(newGate->outputs[i].name, '\0', 100);
				strcpy(newGate->outputs[i].name, token);
				newGate->outputs[i].value = -1;

				if ((token[0] >= 'a') && (token[0] <= 'z')){
					addNode(tempVars, token);
				}
			}
		}
	} else if(strcmp(token, "MULTIPLEXER") == 0) {
		token = strtok(NULL, " ");
		numInput = atoi(token);
		numSelector = (int)(log(numInput)/log(2));
		newGate = createGate(MUX, numInput, 1, numSelector);
		newGate->skipFlag = 0;
		for (i=0;i<numInput;i++){
			token = strtok(NULL, " ");
			if (token[0] == '1'){
				newGate->inputs[i].name = "1";
				newGate->inputs[i].value = 1;
			} else if (token[0] == '0'){
				newGate->inputs[i].name = "0";
				newGate->inputs[i].value = 0;
			} else {
				newGate->inputs[i].name = (char*) malloc(100);
				memset(newGate->inputs[i].name, '\0', 100);
				strcpy(newGate->inputs[i].name, token);
				newGate->inputs[i].value = -1;
			}
		}

		for (i=0;i<numSelector;i++){
			token = strtok(NULL, " \n");
			if (token[0] == '1'){
				newGate->selectorArray[i].name = "1";
				newGate->selectorArray[i].value = 1;
			} else if (token[0] == '0'){
				newGate->selectorArray[i].name = "0";
				newGate->selectorArray[i].value = 0;
			} else {
				newGate->selectorArray[i].name = (char*) malloc(100);
				memset(newGate->selectorArray[i].name, '\0', 100);
				strcpy(newGate->selectorArray[i].name, token);
				newGate->selectorArray[i].value = -1;
			}
		}

		token = strtok(NULL, " \n");
		if (token[0] == '1'){
			newGate->outputs[i].name = "1";
			newGate->outputs[i].value = 1;
		} else if (token[0] == '0'){
			newGate->outputs[0].name = "0";
			newGate->outputs[0].value = 0;
		} else {
			newGate->outputs[0].name = (char*) malloc(100);
			memset(newGate->outputs[0].name, '\0', 100);
			strcpy(newGate->outputs[0].name, token);
			newGate->outputs[0].value = -1;

			if ((token[0] >= 'a') && (token[0] <= 'z')){
				addNode(tempVars, token);
			}
		}
	}

	if (operations == NULL){ //first operation
		operations = newGate;
	} else {
		gate* ptr = operations;
		while (ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = newGate;
	}
	return;
}

void loadGateIO(gate* newGate, char* line, int numInput, int numOutput){
	//input
	int i;
	char* token = strtok(line, " ");
	for (i=0;i<numInput;i++){
		token = strtok(NULL, " ");
		if (token[0] == '1'){
			newGate->inputs[i].name = "1";
			newGate->inputs[i].value = 1;
		} else if (token[0] == '0'){
			newGate->inputs[i].name = "0";
			newGate->inputs[i].value = 0;
		} else {
			newGate->inputs[i].name = (char*) malloc(100);
			memset(newGate->inputs[i].name, '\0', 100);
			strcpy(newGate->inputs[i].name, token);
			newGate->inputs[i].value = -1;
		}
	}
	//output
	for (i=0;i<numOutput;i++){
		token = strtok(NULL, " \n");
		if (token[0] == '1'){
			newGate->outputs[i].name = "1";
			newGate->outputs[i].value = 1;
		} else if (token[0] == '0'){
			newGate->outputs[i].name = "0";
			newGate->outputs[i].value = 0;
		} else {
			newGate->outputs[i].name = (char*) malloc(100);
			memset(newGate->outputs[i].name, '\0', 100);
			strcpy(newGate->outputs[i].name, token);
			newGate->outputs[i].value = -1;

			if ((token[0] >= 'a') && (token[0] <= 'z')){
				addNode(tempVars, token);
			}
		}
	}
	return;
}

gate* createGate(int gateOperation, int numInput, int numOutput, int numSelector){
	gate* newGate = (gate*) malloc(sizeof(gate));
	newGate->type = gateOperation;
	newGate->inputs = (node*) malloc(sizeof(node) * numInput);
	newGate->outputs = (node*) malloc(sizeof(node) * numOutput);
	newGate->selectorArray = (node*) malloc(sizeof(node) * numSelector);
	newGate->numInput = numInput;
	newGate->numOutput = numOutput;
	newGate->numSelector = numSelector;
	newGate->next = NULL;
	return newGate;
}

void addNode(node* head, char* target){
	node* newNode = (node*) malloc(sizeof(node));
	newNode->name = (char*) malloc(100);
	memset(newNode->name,'\0',100);
	strcpy(newNode->name,target);
	newNode->next = NULL;
	newNode->value = -1;
	newNode->skip = 0;

	if (head == NULL){ //first temp node
		tempVars = newNode;
	} else {
		node* ptr = tempVars;
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = newNode;
	}
	return;
}
