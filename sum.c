#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

// structs and typedefs
struct sNode
{
	struct sNode *previous;
	int value;
};
typedef struct sNode Node;
typedef struct
{
	Node *top;
	int count;
} QueueList;
typedef struct t_data
{
	int id;
	int sum;
} threadData;

// Global vars
QueueList *qlValues;
pthread_mutex_t lock;

// Functions declarations
void initProgram(char *path, int numberOfThreads);
void LoadFile(char *path);

void enqueue(QueueList *ql, Node *t);
Node* dequeue(QueueList *ql);
QueueList* newQueueList();
Node* newNode();
void *threadFunc(void *arg);
void runThreads(int number);


// Program functions
int main(int argc, char **argv)
{
	char *path;
	int numberThreads = 1;
	int i;

	if(argc == 2) // soma path
	{
		path = *(argv + 1);
	} 
	if(argc == 4) // soma -t n path
	{
		numberThreads = atoi(*(argv + 2));
		path = *(argv + 3);
	}
	else
	{
		printf("Parametros incorretos.\n");
		exit(0);		
	}

	initProgram(path, numberThreads);

	return 0;
}

void initProgram(char *path, int numberOfThreads)
{
	qlValues = newQueueList();

	// Carrega informacoes do arquivo na Queue List.
	LoadFile(path);
	runThreads(numberOfThreads);
}

void runThreads(int number)
{
	int i, finalSum = 0, rc;
	pthread_t threads[number];
	volatile threadData tdata[number];
	clock_t tstart;
	double tduration;
	
	pthread_mutex_init(&lock, NULL);
	
	tstart = clock();
	for(i = 0; i < number; i++)
	{		
		tdata[i].id = i;
		tdata[i].sum = 0;
		if ((rc = pthread_create(&threads[i], NULL, threadFunc, (void*)&tdata[i])))
		{
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			exit(EXIT_FAILURE);
		}
    }
		
	for(i = 0; i < number; i++)
	{
		pthread_join(threads[i], NULL);
		finalSum += tdata[i].sum;
	}
	tduration = (double)(clock() - tstart)/CLOCKS_PER_SEC;
	
	pthread_mutex_destroy(&lock);
	printf("\ntime: %f\ntotal: %d\n", tduration, finalSum);
}

void *threadFunc(void *arg)
{
	volatile threadData *tdata = (volatile threadData*) arg;
	
	pthread_mutex_lock(&lock);
	do
	{
		Node *node = NULL;
		
		if((node = dequeue(qlValues)) != NULL)
		{
			tdata->sum += node->value;
			free(node);
			pthread_mutex_unlock(&lock);
		}
		else
		{
			pthread_mutex_unlock(&lock);
			pthread_exit();
			return EXIT_SUCCESS;
		}
			
		pthread_mutex_lock(&lock);
	} while(qlValues->count > 0);
	
	pthread_mutex_unlock(&lock);
	pthread_exit();
}

void LoadFile(char *path)
{
	FILE *file;
	int i = 0;
	char c;
	char buf[10];

	file = fopen(path, "r");
	if(file == NULL)
	{
		printf("Arquivo nao encontrado.\n");
		exit(0);
	}

	while(1)
	{
		c = fgetc(file);
		
		if(c == ' ')
		{
			int j;
			Node *node = newNode();
			node->value = atoi(buf);
			enqueue(qlValues, node);
			// Limpa o buffer
			i = 0;
			for(j = 0; j < 10; j++)
				buf[j] = '\0';

			continue;
		}
		else if(c >= '0' && c <= '9')
		{
			buf[i++] = c;
			continue;
		}
		else if(c == 0 || c == '\n') // Evita o problema de aparecer misteriosos \n no final dos arquivos. (Provavelmente coisa do editor)
		{
			continue;
		}
		else if(c == EOF)
		{
			printf("\n");
			return;
		}
		else
		{
			printf("Erro para %s em '%c'.\n", path, c);
			exit(0);
		}
	}
}

// Linked Queue List functions

void enqueue(QueueList *ql, Node *t)
{
	if(t == NULL)
		return;

	// Verifica lista vazia
	if(ql->top == NULL)
	{
		t->previous = NULL;
		ql->top = t;
		ql->count++;
		return;
	}

	t->previous = ql->top;
	ql->top = t;
	ql->count++;
}

Node* dequeue(QueueList *ql)
{
	Node *dqNode;

	if(ql->count == 0)
	{
		ql->top = NULL;
		return NULL;
	}
	else if(ql->count == 1)
	{
		dqNode = ql->top;
		ql->top = NULL;
		ql->count = 0;
	}
	else
	{	
		dqNode = ql->top;
		ql->top = ql->top->previous;
		ql->count--;
	}
	
	return dqNode;
}

QueueList* newQueueList()
{
	QueueList *ql = (QueueList*) malloc(sizeof(QueueList));

	ql->top = NULL;
	ql->count = 0;

	return ql;
}

Node* newNode()
{
	Node *node = (Node*) malloc(sizeof(Node));
	
	node->previous = NULL;
	node->value = 0;

	return node;
}
