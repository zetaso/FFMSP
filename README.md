# FFMSP
GRASP and Genetic algorithms for the "Far From Most String Problem".


## DESCRIPCION
	El programa debe compilarse y ejecutarse 
	como sigue:

	$ g++ ffmsp.cpp -o grasp
	$ ./grasp -i [XXX-YYY-ZZZ] -th [tr] -p [eps] -t [t]

	donde:
		X es la cantidad de filas
		Y es la cantidad de columnas
		Z es la cantidad de instancias
		tr es el treshold (flotante), con tr entre [0.0, 1.0]
		eps es epsilon (flotante), con eps entre [0.0, 1.0]
		t es el tiempo en segundos, con t > 0.

### Ejemplo:
	----------INPUT----------
	Para 100 filas, 300 columnas y la instancia 001.

	$ ./grasp -i 100-300-001 -th 0.85 -p 0.1 -t 10.0


	----------OUTPUT----------
	Devuelve la métrica de la mejor 
	solución encontrada en el tiempo
	calculado.
