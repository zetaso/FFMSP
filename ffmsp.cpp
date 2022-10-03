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

int accumulated_score;
int best_score = -1;
char *best_solution;

int *col_indexes;		//indices de columnas en el orden a revisar
bool *row_validity;		//viabilidad de los string para cumplir
int *faults;			//array que contiene la cantidad de faltas (caracteres iguales a la solución) de cada string
int *hits;				//array que contiene la cantidad de aciertos (caracteres distintos a la solución) de cada string

vector<char> *empatados;

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

void local_search(char** sequences, int rows, int columns, char* solution)
{
	int start = rand() % columns;
	int end = (start + rand() % columns) % columns;


	//	AL DESCOMENTAR (y debugear) SE RECONSTRUIRÁ ESTA PARTE DE LA SOLUCION EN ORDEN ALEATORIO
	//	FIXEAR ESTE CODIGO:
	/*
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
	*/

	// Revertimos la métrica
	
	for(int j = start; j != end; j = (j + 1) % columns)
	{
		for(int i = 0; i < rows; i++)
		{
			if(sequences[i][j] == solution[j])
			{
				faults[i]--;
				if(!row_validity[i] && hits[i] < char_goal && faults[i] <= columns - char_goal)
					row_validity[i] = true;	//vuelven a ser vigentes los indices de strings ya no listos ni descartados
			}
			else
			{
				hits[i]--;
				if(!row_validity[i] && hits[i] < char_goal && faults[i] <= columns - char_goal)
				{
					row_validity[i] = true;	//vuelven a ser vigentes los indices de strings ya no listos ni descartados
					accumulated_score--;
				}
			}
		}
	}

	//edición de la solución, columna por columna
	for(int j = start; j != end; j = (j + 1) % columns)
	{
		bool found_by_metric = false;
		if(j >= char_goal)
		{
			int scores[4];
			for(int k = 0; k < 4; k++)
			{
				scores[k] = accumulated_score;
				for(int i = 0; i < rows; i++)
					if(row_validity[i] && alphabet[k] != sequences[i][col_indexes[j]] && hits[i] >= char_goal - 1)
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
			for(int i = 0; i < rows; i++)
				if(row_validity[i])
					switch(sequences[i][col_indexes[j]])
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
		for(int i = 0; i < rows; i++)
			if(row_validity[i] && sequences[i][col_indexes[j]] == solution[col_indexes[j]] && ++faults[i] > columns - char_goal)
				row_validity[i] = false;
			else if(row_validity[i] && sequences[i][col_indexes[j]] != solution[col_indexes[j]] && ++hits[i] == char_goal)
			{
				row_validity[i] = false;
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

/* 
	Recorre las posiciones j y cambia las posiciones que empataron por otra opción disponible
*/
void local_search_empates(char** sequences, int rows, int columns, char* solution){
	for(int j = 0; j < columns; j++){
		solution[j] = empatados[j].at(0);
		empatados[j].erase(empatados[j].begin());
	}
}

void greedy(char** sequences, int rows, int columns, char* solution)
{
	//randomizar la forma en que se revisan las columnas
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
		row_validity[i] = true;
		faults[i] = 0;
		hits[i] = 0;
	}

	//creación de una solución, columna por columna
	accumulated_score = 0;
	for(int j = 0; j < columns; j++)
	{
		bool found_by_metric = false;
		if(j >= char_goal)
		{
			int scores[4];
			for(int k = 0; k < 4; k++)
			{
				scores[k] = accumulated_score;
				for(int i = 0; i < rows; i++)
					if(row_validity[i] && alphabet[k] != sequences[i][col_indexes[j]] && hits[i] >= char_goal - 1)
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
			for(int i = 0; i < rows; i++)
				if(row_validity[i])
					switch(sequences[i][col_indexes[j]])
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
			for(int i = 0; i < equals; i++){
				if(i != selected){
					empatados[col_indexes[j]].push_back(alphabet[lowests[i]]);
				}
			}
			solution[col_indexes[j]] = alphabet[lowests[selected]];
		}
		for(int i = 0; i < rows; i++)
			if(row_validity[i] && sequences[i][col_indexes[j]] == solution[col_indexes[j]] && ++faults[i] > columns - char_goal)
				row_validity[i--] = false;
			else if(row_validity[i] && sequences[i][col_indexes[j]] != solution[col_indexes[j]] && ++hits[i] == char_goal)
			{
				row_validity[i--] = false;
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
	//randomizar la forma en que se revisan las columnas
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
		row_validity[i] = true;
		faults[i] = 0;
		hits[i] = 0;
	}

	//creación de una solución, columna por columna
	accumulated_score = 0;
	for(int j = 0; j < columns; j++)
	{
		double random = rand() / double(RAND_MAX);
		if(random <= (double) prob)
			solution[col_indexes[j]] = alphabet[rand() % 4];
		else
		{
			bool found_by_metric = false;
			if(j >= char_goal)
			{
				int scores[4];
				for(int k = 0; k < 4; k++)
				{
					scores[k] = accumulated_score;
					for(int i = 0; i < rows; i++)
						if(row_validity[i] && alphabet[k] != sequences[i][col_indexes[j]] && hits[i] >= char_goal - 1)
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
				for(int i = 0; i < rows; i++)
					if(row_validity[i])
						switch(sequences[i][col_indexes[j]])
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
		for(int i = 0; i < rows; i++)
			if(row_validity[i] && sequences[i][col_indexes[j]] == solution[col_indexes[j]] && ++faults[i] > columns - char_goal)
				row_validity[i--] = false;
			else if(row_validity[i] && sequences[i][col_indexes[j]] != solution[col_indexes[j]] && ++hits[i] == char_goal)
			{
				row_validity[i--] = false;
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
	for(int j = 0; j < columns; j++)
		col_indexes[j] = j;

	row_validity = (bool*) malloc(rows * sizeof(bool));
	hits = (int*) malloc(columns * sizeof(int));
	faults = (int*) malloc(columns * sizeof(int));

	char **sequences = (char**) malloc(rows * sizeof(char*));
	for (int i = 0; i < rows; ++i)
		sequences[i] = (char*) malloc(columns * sizeof(char));

	char *solution = (char*) malloc(columns * sizeof(char));
	best_solution = (char*) malloc(columns * sizeof(char));

	empatados = (vector<char>*) malloc(columns * sizeof(vector<char>));

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

		local_search(sequences, rows, columns, solution);

		clock_t end = clock();
		total = (float) (end - start) / CLOCKS_PER_SEC;
	}

	cout << "metrica: " << best_score << endl;

	return 0;
}