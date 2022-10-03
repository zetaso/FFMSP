# FFMSP
GRASP and Genetic algorithms for the "Far From Most String Problem".


## DESCRIPCION
	El programa debe compilarse y ejecutarse 
	como sigue:

	$ g++ ffmsp.cpp -o grasp
	$ ./grasp -i [XXX-YYY-ZZZ].txt -th [tr] [eps] [t]

	donde:
		X es la cantidad de filas
		Y es la cantidad de columnas
		Z es la cantidad de instancias
		tr es el treshold (flotante)
		eps es epsilon (flotante)
		t es el tiempo en segundos

### Ejemplo:
	----------INPUT----------
	Para 100 filas, 300 columnas y 
	la instancia 001.

	$ ./grasp -i 100-300-001.txt -th 0.85 0.1 10


	----------OUTPUT----------
	Devuelve la métrica de la mejor 
	solución encontrada en el tiempo
	calculado.
