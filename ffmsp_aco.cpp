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

float base_pheromone = 1;
float delta_pheromone = 0.2;

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

void copy_into(char* src, char* dst, int columns)
{
	for (int i = 0; i <= columns; i++)
		dst[i] = src[i];
}

void aco(char** sequences, int rows, int columns, float **pheromones, int prows, char **ants, int ants_count, float alpha, float beta, float rho, char* solution)
{
	// valor inicial feromonas
	for(int i = 0; i < prows; i++)
		for (int j = 0; j < columns; ++j)
		pheromones[i][j] = base_pheromone;

	// posicionar hormigas aleatoriamente
	for (int i = 0; i < ants_count; ++i)
	{
		int r = rand() % 4;
		ants[i][0] = alphabet[r];
		// actualizar feromona en esa posición
		pheromones[r][0] += delta_pheromone;
	}

	// ciclo de steps
	for (int j = 1; j < columns; ++j)
	{
		// calculo de la heurística

			// contar la aparicion de cada letra
		int instances[] = {0, 0, 0, 0};
		for(int i = 0; i < rows; i++)
			switch(sequences[i][j])
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

			// conversion de apariciones a heuristica para cada letra
		float h_value[prows];
		for(int i = 0; i < prows; i++)
			h_value[i] = rows / (float) instances[i];

			// calculo de la suma de pheromone * heuristic (DENOMINADOR)
		float sum = 0;
		for(int i = 0; i < prows; i++)
			sum += pow(pheromones[i][j], alpha) * pow(h_value[i], beta);

			// asignación de la probabilidad acumulada de cada letra
		float probabilities[prows][2];
		for (int i = 0; i < prows; ++i)
		{
			// calculo del valor pheromone * heuristic de cada letra (NUMERADOR)
			float prob = pow(pheromones[i][j], alpha) * pow(h_value[i], beta) / sum;
			probabilities[i][0] = i > 0 ? probabilities[i - 1][1] : 0;
			probabilities[i][1] = probabilities[i][0] + prob;
		}

		// por cada hormiga
		for (int h = 0; h < ants_count; ++h)
		{
			double random = rand() / double(RAND_MAX);
			// por cada letra
			for (int i = 0; i < prows; ++i)
				if(random <= (double) probabilities[i][1])
				{
					ants[h][j] = alphabet[i];
					// actualizar feromona en esa posición
					pheromones[i][j] += 0.1;
					break;
				}
		}
	}
	
	// actualizar feromonas
	for (int i = 0; i < prows; ++i)
		for (int j = 0; j < columns; ++j)
		{
			pheromones[i][j] *= (1 - rho) * pheromones[i][j] + rho * base_pheromone;
		}

	// actualizar solucion actual
	int best_ant = 0;
	int best_score = check_score(sequences, rows, columns, ants[best_ant]);
	for (int h = 1; h < ants_count; ++h)
	{
		int current_score = check_score(sequences, rows, columns, ants[h]);
		if(current_score > best_score)
		{
			best_ant = h;
			best_score = current_score;
		}
	}
	copy_into(ants[best_ant], solution, columns);
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

	for(int i = 5; i < argc-1; i+=2){
		if(strcmp(argv[i], "-t") == 0){
			run_time = stof(argv[i+1]);
		}
	}

	char **sequences = (char**) malloc(rows * sizeof(char*));
	for (int i = 0; i < rows; ++i)
		sequences[i] = (char*) malloc(columns * sizeof(char));

	char *best_solution = (char*) malloc(columns * sizeof(char));
	char *curr_solution = (char*) malloc(columns * sizeof(char));

	float **pheromones = (float**) malloc(4 * sizeof(float*));
	for (int i = 0; i < 4; ++i)
		pheromones[i] = (float*) malloc(columns * sizeof(float));

	int ant_count = 50;
	float alpha = 1;
	float beta = 1;
	float rho = 0.1;
	
	char **ants = (char**) malloc(ant_count * sizeof(char*));
	for (int i = 0; i < rows; ++i)
		ants[i] = (char*) malloc(columns * sizeof(char));


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

	while(total < run_time)
	{
		aco(sequences, rows, columns, pheromones, 4, ants, ant_count, alpha, beta, rho, curr_solution);

		clock_t end = clock();
		total = (float) (end - start) / CLOCKS_PER_SEC;

		int curr_score = check_score(sequences, rows, columns, curr_solution);
		if(curr_score > last_score)
		{
			best_time = total;
			cout << (int) curr_score << " - " << fixed << setprecision(2) << total << endl;
			last_score = (int) curr_score;
			copy_into(curr_solution, best_solution, columns);
		}
	}

	free(col_indexes);
	free(row_validity);
	free(hits);
	free(faults);

	for (int i = 0; i < rows; ++i)
		free(sequences[i]);
	free(sequences);

	return 0;
}