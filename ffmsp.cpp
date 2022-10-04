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
int last_score = -1;

int *col_indexes;		//indices de columnas en el orden a revisar
bool *row_validity;		//viabilidad de los string para cumplir
int *faults;			//array que contiene la cantidad de faltas (caracteres iguales a la solución) de cada string
int *hits;				//array que contiene la cantidad de aciertos (caracteres distintos a la solución) de cada string

vector<char> *empatados;

int check_args(int argc, char* argv[])
{
	if(argc < 5)
	{
		cout << "Error: muy pocos parametros" << endl;
		return 1;
	}

	if(strcmp(argv[1], "-i"))
	{
		cout << "Error: no hay instancia" << endl;
		return 1;
	}

	if(strcmp(argv[3], "-th"))
	{
		cout << "Error: no hay threshold" << endl;
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
	int diff = 0;

	for(int i = 0; i < rows; i++)
	{
		diff = 0;
		for(int j = 0; j < columns; j++)
			if(sequences[i][col_indexes[j]] != solution[col_indexes[j]])
				diff++;
		if(diff >= char_goal)
			score++;
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
				if(!row_validity[i] && hits[i] < char_goal && hits[i] < char_goal)
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
}

/* 
	Recorre las posiciones j y cambia las posiciones que empataron por otra opción disponible
*/
void local_search_empates(char** sequences, int rows, int columns, char* solution)
{
	for(int j = 0; j < columns; j++)
	{
		if(empatados[col_indexes[j]].size() > 0)
		{
			for(int i = 0; i < rows; i++)
			{
				if(sequences[i][col_indexes[j]] == solution[col_indexes[j]])
				{
					faults[i]--;
					if(!row_validity[i] && hits[i] < char_goal && faults[i] <= columns - char_goal)
						row_validity[i] = true;	//vuelven a ser vigentes los indices de strings ya no listos ni descartados
				}
				else
				{
					hits[i]--;
					if(!row_validity[i] && hits[i] < char_goal && hits[i] < char_goal)
					{
						row_validity[i] = true;	//vuelven a ser vigentes los indices de strings ya no listos ni descartados
						accumulated_score--;
					}
				}
			}

			solution[col_indexes[j]] = empatados[col_indexes[j]].at(0);
			empatados[col_indexes[j]].erase(empatados[col_indexes[j]].begin());

			for(int i = 0; i < rows; i++)
				if(row_validity[i] && sequences[i][col_indexes[j]] == solution[col_indexes[j]])
				{
					faults[i] = faults[i] + 1;
					if(faults[i] > columns - char_goal)
						row_validity[i] = false;
				}
				else if(row_validity[i] && sequences[i][col_indexes[j]] != solution[col_indexes[j]])
				{
					hits[i] = hits[i] + 1;
					if(hits[i] == char_goal)
					{
						row_validity[i] = false;
						accumulated_score++;
					}
				}
		}
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
			int upgrades[4];
			for(int k = 0; k < 4; k++)
			{
				upgrades[k] = 0;
				for(int i = 0; i < rows; i++)
					if(row_validity[i] && alphabet[k] != sequences[i][col_indexes[j]] && hits[i] + 1 >= char_goal)
						upgrades[k]++;
			}
			int highest_index = 0;
			for(int i = 1; i < 4; i++)
				if(upgrades[i] > upgrades[highest_index])
					highest_index = i;
			int highests[4];
			int equals = 0;
			for(int i = 0; i < 4; i++)
				if(upgrades[i] == upgrades[highest_index])
				{
					highests[equals] = i;
					equals++;
				}
			if(upgrades[highest_index] > 0)
			{
				int selected = rand() % equals;
				solution[col_indexes[j]] = alphabet[highests[selected]];
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

			empatados[col_indexes[j]].clear();
			for(int i = 0; i < equals; i++)
				if(i != selected)
					empatados[col_indexes[j]].push_back(alphabet[lowests[i]]);

			solution[col_indexes[j]] = alphabet[lowests[selected]];
		}

		for(int i = 0; i < rows; i++)
			if(row_validity[i] && sequences[i][col_indexes[j]] == solution[col_indexes[j]])
			{
				faults[i] = faults[i] + 1;
				if(faults[i] == columns - char_goal + 1)
					row_validity[i] = false;
			}
			else if(row_validity[i] && sequences[i][col_indexes[j]] != solution[col_indexes[j]])
			{
				hits[i] = hits[i] + 1;
				if(hits[i] == char_goal)
				{
					row_validity[i] = false;
					accumulated_score++;
				}
			}
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
						if(row_validity[i] && alphabet[k] != sequences[i][col_indexes[j]] && hits[i] == char_goal - 1)
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
				row_validity[i] = false;
			else if(row_validity[i] && sequences[i][col_indexes[j]] != solution[col_indexes[j]] && ++hits[i] == char_goal)
			{
				row_validity[i] = false;
				accumulated_score++;
			}
	}
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	clock_t start = clock();

	if(check_args(argc, argv))
		return 1;

	int rows, columns;
	parse_input(argv[2], &rows, &columns);

	char_goal = int(stof(argv[4]) * columns);

	float run_time = 0.1;
	float probability = 0;
	method = 0;

	for(int i = 5; i < argc-1; i+=2){

		if(strcmp(argv[i], "-p") == 0){
			probability = stof(argv[i+1]);
			if(probability > 0)
				method = 1;
		}
		else if(strcmp(argv[i], "-t") == 0){
			run_time = stof(argv[i+1]);
		}
	}

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

	empatados = new vector<char>[columns];

	alphabet[0] = 'A';
	alphabet[1] = 'C';
	alphabet[2] = 'G';
	alphabet[3] = 'T';

	string instance_name(argv[2]);
	fstream file("datasets/" + instance_name + ".txt");

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

		if(accumulated_score > last_score)
		{
			cout << accumulated_score << " - " << fixed << setprecision(2) << total << endl;
			last_score = accumulated_score;
		}

		local_search_empates(sequences, rows, columns, solution);

		end = clock();
		total = (float) (end - start) / CLOCKS_PER_SEC;

		if(accumulated_score > last_score)
		{
			cout << accumulated_score << " - " << fixed << setprecision(2) << total << endl;
			last_score = accumulated_score;
		}
	}

	free(col_indexes);
	free(row_validity);
	free(hits);
	free(faults);

	for (int i = 0; i < rows; ++i)
		free(sequences[i]);
	free(sequences);

	free(solution);
	free(empatados);

	return 0;
}