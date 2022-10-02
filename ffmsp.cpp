#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <vector>
#include <cmath>
#include <time.h>

using namespace std;

char method;
char alphabet[4];
int char_goal;

int best_score = -1;
char *best_solution;

int *col_indexes;

int check_args(int argc, char* argv[])
{
	if(argc < 5)
	{
		cout << "error: muy pocos parametros" << endl;
		return 1;
	}
	else if(strcmp(argv[1], "-i"))
	{
		cout << "error: no hay instancia" << endl;
		return 1;
	}
	else if(strcmp(argv[3], "-th"))
	{
		cout << "error: no hay threshold" << endl;
		return 1;
	}
	return 0;
}

void parse_input(char* instance, int* n, int* m)
{
	int* vars[] = {n, m};
	int var = 0;

	char* value = new char(3);
	int value_index = 0;

	int comma = 0;
	for(int i = 0; i < 15 && comma != 2; i++) {
		if(instance[i] == '-')
		{
			vars[var][0] = stoi(value);
			var++;
			value = new char(3);
			value_index = 0;
			comma++;
			continue;
		}
		value[value_index++] = instance[i];
	}
}

int check_score(char** sequences, int rows, int columns, char* solution)
{
	int score = 0;
	int dif = 0;

	for(int i = 0; i < rows; i++)
	{
		dif = 0;
		for(int j = 0; j < columns; j++)
		{
			if(sequences[i][j] != solution[j])
			{
				dif++;
				if(dif == char_goal)
					score++;
			}
		}
	}
	return score;
}
	
void greedy(char** sequences, int rows, int columns, char* solution)
{
	vector<int> indexes;	//indices de strings que aun pueden cumplir
	int faults[rows];		//array que contiene la cantidad de faltas (caracteres iguales a la solución) de cada string
	int hits[rows];			//array que contiene la cantidad de aciertos (caracteres distintos a la solución) de cada string

	//randomizar la forma en que se revisan las columnas
	for(int j = 0; j < columns; j++)
		col_indexes[j] = j;
	for(int j = 0; j < columns; j++)
	{
		int rand_index = j + rand() % (columns - j);
		int aux = col_indexes[j];
		col_indexes[j] = col_indexes[rand_index];
		col_indexes[rand_index] = aux;
	}

	//inicialización
	for(int i = 0; i < rows; i++)
	{
		indexes.push_back(i);	//se inicializa con los indices de todos los strings
		faults[i] = 0;
		hits[i] = 0;
	}

	//creación de una solución, columna por columna
	int accumulated_score = 0;
	for(int j = 0; j < columns; j++)
	{
		bool found_by_metric = false;
		int actual_rows = indexes.size();
		if(j >= char_goal)
		{
			int scores[4];
			for(int k = 0; k < 4; k++)
			{
				scores[k] = accumulated_score;
				for(int i = 0; i < actual_rows; i++)
					if(alphabet[k] != sequences[indexes[i]][col_indexes[j]] && hits[indexes[i]] >= char_goal - 1)
						scores[k]++;
			}
			int highest_index = 0;
			for(int i = 1; i < 4; i++)
				if(scores[i] > scores[highest_index])
					highest_index = i;
			if(scores[highest_index] > accumulated_score)
			{
				solution[col_indexes[j]] = alphabet[highest_index];
				found_by_metric = true;
			}
		}
		if(!found_by_metric)
		{
			int instances[] = {0, 0, 0, 0};
			for(int i = 0; i < actual_rows; i++)
				switch(sequences[indexes[i]][col_indexes[j]])
				{
					case 'A':
						instances[0]++;
						break;
					case 'C':
						instances[1]++;
						break;
					case 'G':
						instances[2]++;
						break;
					case 'T':
						instances[3]++;
						break;
				}
			int lowest_index = 0;
			for(int i = 1; i < 4; i++)
				if(instances[i] < instances[lowest_index])
					lowest_index = i;
			int lowests[4];
			int equals = 0;
			for(int i = 0; i < 4; i++)
				if(instances[i] == instances[lowest_index])
				{
					lowests[equals] = i;
					equals++;
				}
			int selected = rand() % equals;
			solution[col_indexes[j]] = alphabet[lowests[selected]];
		}
		for(int i = 0; i < indexes.size(); i++)
			if(sequences[indexes[i]][col_indexes[j]] == solution[col_indexes[j]] && ++faults[indexes[i]] > columns - char_goal)
				indexes.erase(indexes.begin() + i--);
			else if(sequences[indexes[i]][col_indexes[j]] != solution[col_indexes[j]] && ++hits[indexes[i]] == char_goal)
			{
				indexes.erase(indexes.begin() + i--);
				accumulated_score++;
			}
	}

	if(accumulated_score > best_score)
	{
		best_score = accumulated_score;
		for(int j = 0; j < columns; j++)
			best_solution[j] = solution[j];
	}
}
	
void pgreedy(char** sequences, int rows, int columns, char* solution, float prob)
{
	vector<int> indexes;	//indices de strings que aun pueden cumplir
	int faults[rows];		//array que contiene la cantidad de faltas (caracteres iguales a la solución) de cada string
	int hits[rows];			//array que contiene la cantidad de aciertos (caracteres distintos a la solución) de cada string

	//randomizar la forma en que se revisan las columnas
	for(int j = 0; j < columns; j++)
		col_indexes[j] = j;
	for(int j = 0; j < columns; j++)
	{
		int rand_index = j + rand() % (columns - j);
		int aux = col_indexes[j];
		col_indexes[j] = col_indexes[rand_index];
		col_indexes[rand_index] = aux;
	}

	//inicialización
	for(int i = 0; i < rows; i++)
	{
		indexes.push_back(i);	//se inicializa con los indices de todos los strings
		faults[i] = 0;
		hits[i] = 0;
	}

	//creación de una solución, columna por columna
	int accumulated_score = 0;
	for(int j = 0; j < columns; j++)
	{
		double random = rand() / double(RAND_MAX);
		if(random <= (double) prob)
			solution[col_indexes[j]] = alphabet[rand() % 4];
		else
		{
			bool found_by_metric = false;
			int actual_rows = indexes.size();
			if(j >= char_goal)
			{
				int scores[4];
				for(int k = 0; k < 4; k++)
				{
					scores[k] = accumulated_score;
					for(int i = 0; i < actual_rows; i++)
						if(alphabet[k] != sequences[indexes[i]][col_indexes[j]] && hits[indexes[i]] >= char_goal - 1)
							scores[k]++;
				}
				int highest_index = 0;
				for(int i = 1; i < 4; i++)
					if(scores[i] > scores[highest_index])
						highest_index = i;
				if(scores[highest_index] > accumulated_score)
				{
					solution[col_indexes[j]] = alphabet[highest_index];
					found_by_metric = true;
				}
			}
			if(!found_by_metric)
			{
				int instances[] = {0, 0, 0, 0};
				for(int i = 0; i < actual_rows; i++)
					switch(sequences[indexes[i]][col_indexes[j]])
					{
						case 'A':
							instances[0]++;
							break;
						case 'C':
							instances[1]++;
							break;
						case 'G':
							instances[2]++;
							break;
						case 'T':
							instances[3]++;
							break;
					}
				int lowest_index = 0;
				for(int i = 1; i < 4; i++)
					if(instances[i] < instances[lowest_index])
						lowest_index = i;
				int lowests[4];
				int equals = 0;
				for(int i = 0; i < 4; i++)
					if(instances[i] == instances[lowest_index])
					{
						lowests[equals] = i;
						equals++;
					}
				int selected = rand() % equals;
				solution[col_indexes[j]] = alphabet[lowests[selected]];
			}
		}
		for(int i = 0; i < indexes.size(); i++)
			if(sequences[indexes[i]][col_indexes[j]] == solution[col_indexes[j]] && ++faults[indexes[i]] > columns - char_goal)
				indexes.erase(indexes.begin() + i--);
			else if(sequences[indexes[i]][col_indexes[j]] != solution[col_indexes[j]] && ++hits[indexes[i]] >= char_goal)
			{
				indexes.erase(indexes.begin() + i--);
				accumulated_score++;
			}
	}

	if(accumulated_score > best_score)
	{
		best_score = accumulated_score;
		for(int j = 0; j < columns; j++)
			best_solution[j] = solution[j];
	}
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	clock_t start = clock();

	if(check_args(argc, argv))
		return 1;

	float run_time = 0.1;
	float probability = 0;
	method = 0;

	if(argc >= 6)
	{
		probability = stof(argv[5]);
		if(probability > 0)
			method = 1;
	}

	if(argc >= 7)
		run_time = stof(argv[6]);

	int rows, columns;
	parse_input(argv[2], &rows, &columns);

	char_goal = int(stof(argv[4]) * columns);

	col_indexes = (int*) malloc(columns * sizeof(int));
	char **sequences = (char**) malloc(rows * sizeof(char*));
	for (int i = 0; i < columns; ++i)
	{
		sequences[i] = (char*) malloc(columns * sizeof(char));
		col_indexes[i] = i;
	}
	char *solution = (char*) malloc(columns * sizeof(char));
	best_solution = (char*) malloc(columns * sizeof(char));

	alphabet[0] = 'A';
	alphabet[1] = 'C';
	alphabet[2] = 'G';
	alphabet[3] = 'T';

	string instance_name(argv[2]);
	fstream file("datasets/" + instance_name);

	string current;
	for(int i = 0; getline(file, current) && i < rows; i++)
		for(int j = 0; j < current.length(); j++)
			sequences[i][j] = current.at(j);

	file.close();

	float total = 0;

	if(method == 0)
		cout << "greedy" << endl;
	else
		cout << "pgreedy" << endl;

	while(total < run_time)
	{
		if(method == 0)
			greedy(sequences, rows, columns, solution);
		else
			pgreedy(sequences, rows, columns, solution, probability);

		clock_t end = clock();
		total = (float) (end - start) / CLOCKS_PER_SEC;
	}

	cout << "metrica: " << best_score << endl;

	return 0;
}