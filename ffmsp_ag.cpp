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
int tuning = 0;
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
			if(sequences[i][j] != solution[j])
			{
				diff++;
				if(diff >= char_goal)
				{
					score++;
					break;
				}
			}
	}
	return score;
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
	solution[columns] = accumulated_score;
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
	solution[columns] = accumulated_score;
}

int best_of(int first_id, char* first, int second_id, char* second, int columns)
{
	// por optimizar, la métrica de cada solución se almacena
	// en la última posición del array que la representa
	if(first[columns] > second[columns])
		return first_id;
	else if(first[columns] < second[columns])
		return second_id;
	else
		return (rand() % 2 == 0) ? first_id : second_id;
}

void recombine(char* first, char* second, char* first_child, char* second_child, int columns)
{
	int pivot = rand() % columns;
	for (int i = 0; i < columns; i++)
	{
		if(i < pivot)
		{
			first_child[i] = first[i];
			second_child[i] = second[i];
		}
		else
		{
			first_child[i] = second[i];
			second_child[i] = first[i];
		}
	}
}

void copy_into(char* src, char* dst, int columns)
{
	for (int i = 0; i <= columns; i++)
		dst[i] = src[i];
}

void mutate(char* solution, int columns)
{
	int rand_pos = rand() % columns;
	solution[rand_pos] = alphabet[rand() % 4];
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
	int population_size = 0;
	method = 0;

	for(int i = 5; i < argc-1; i+=2){

		if(strcmp(argv[i], "-p") == 0){
			probability = stof(argv[i+1]);
			if(probability > 0)
				method = 1;
		}
		else if(strcmp(argv[i], "-ms") == 0){
			population_size = stof(argv[i+1]);
			if(population_size < 4)
				population_size = 4;
		}
		else if(strcmp(argv[i], "-t") == 0){
			run_time = stof(argv[i+1]);
		}
		else if(strcmp(argv[i], "-tuning") == 0){
			tuning = atoi(argv[i+1]);
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

	char **solutions = (char**) malloc(population_size * sizeof(char*));
	for (int i = 0; i < population_size; ++i)
		solutions[i] = (char*) malloc((1 + columns) * sizeof(char));

	char **children = (char**) malloc(2 * sizeof(char*));
	for (int i = 0; i < 2; ++i)
		children[i] = (char*) malloc((1 + columns) * sizeof(char));

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

	int best_index;
	float best_time;

	for (int i = 0; i < population_size; ++i)
	{
		if(method == 0)
			greedy(sequences, rows, columns, solutions[i]);
		else
			pgreedy(sequences, rows, columns, solutions[i], probability);

		clock_t end = clock();
		total = (float) (end - start) / CLOCKS_PER_SEC;

		if(accumulated_score > last_score)
		{
			last_score = accumulated_score;
			best_index = i;
			best_time = total;
		}
	}
	
	cout << (int) solutions[best_index][columns] << " - " << fixed << setprecision(2) << best_time << endl;

	while(total < run_time)
	{
		int parents[4];
		for (int i = 0; i < 4; i++)
		{
			bool different = true;
			do
			{
				parents[i] = rand() % population_size;
				for (int j = 0; j < i; j++)
				{
					if(parents[i] == parents[j])
					{
						different = true;
						break;
					}
				}
			}
			while(!different);
		}

		int bests[2];
		bests[0] = best_of(parents[0], solutions[parents[0]], parents[1], solutions[parents[1]], columns);
		bests[1] = best_of(parents[2], solutions[parents[2]], parents[3], solutions[parents[3]], columns);

		recombine(solutions[bests[0]], solutions[bests[1]], children[0], children[1], columns);
		
		double random = rand() / double(RAND_MAX);
		if(random <= (double) 0.15)
			mutate(children[0], columns);
		
		random = rand() / double(RAND_MAX);
		if(random <= (double) 0.15)
			mutate(children[1], columns);

		children[0][columns] = check_score(sequences, rows, columns, children[0]);
		children[1][columns] = check_score(sequences, rows, columns, children[1]);

		int first_worst = 0;
		int second_worst = 1;
		for (int i = 2; i < population_size; i++)
		{
			if(solutions[i][columns] > solutions[first_worst][columns])
			{
				if(solutions[first_worst][columns] > solutions[second_worst][columns])
					second_worst = first_worst;
				first_worst = i;
			}
			else if(solutions[i][columns] > solutions[second_worst][columns])
			{
				if(solutions[second_worst][columns] > solutions[first_worst][columns])
					first_worst = second_worst;
				second_worst = i;
			}
		}

		if(children[0][columns] > solutions[first_worst][columns])
			copy_into(children[0], solutions[first_worst], columns);

		if(children[1][columns] > solutions[second_worst][columns])
			copy_into(children[1], solutions[second_worst], columns);

		clock_t end = clock();
		total = (float) (end - start) / CLOCKS_PER_SEC;

		if(solutions[first_worst][columns] > last_score)
		{
			best_index = first_worst;
			best_time = total;
			cout << (int) solutions[first_worst][columns] << " - " << fixed << setprecision(2) << total << endl;
			last_score = (int) solutions[first_worst][columns];
		}
		if(solutions[second_worst][columns] > last_score)
		{
			best_index = first_worst;
			best_time = total;
			cout << (int) solutions[second_worst][columns] << " - " << fixed << setprecision(2) << total << endl;
			last_score = (int) solutions[second_worst][columns];
		}
	}

	cout << endl << (int) solutions[best_index][columns] << " - " << fixed << setprecision(2) << best_time << endl;
	for (int i = 0; i < columns; i++)
	{
		cout << solutions[best_index][i];
	}
	cout << endl;

	free(col_indexes);
	free(row_validity);
	free(hits);
	free(faults);

	for (int i = 0; i < rows; ++i)
		free(sequences[i]);
	free(sequences);

	for (int i = 0; i < population_size; ++i)
		free(solutions[i]);
	free(solutions);

	return 0;
}