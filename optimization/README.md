# Computer Systems Architecture -- Assignment 2

## blas
	- in implementare m-am folosit de functia cblas_dtrmm, care realizeaza inmultirea matricilor tinand cont de ordine, de existenta matricelor lower/upper triangular & de transpunere;
   	- mi-am mai definit o functie de copiere, copy_mat, care sa ma ajute sa pastrez valorile initiale si dupa apelarea functiei din blas

	   - am apelat cblas_dtrmm de 3 ori:
		1) pentru B * A_transpose: 
		cblas_dtrmm(101, // row-major
			    142, // matricea descrisa se afla pe partea dreapta
			    121, // matricea este upper-triangular
			    112, // avem nevoie de transpusa
			    131, // non-unit matrix 
			    N, N, // nr linii & coloane
			    1, // scalar
			    A, // matricea initiala A, ce va fi convertita in transpusa si inmultita cu B
			    N, // precizie A
			    result, // a doua matrice din operatie, in care va fi retinut rezultatul; in ea se copiaza valorile din B inainte de apelarea functiei
			    N); // precizie B

		2) pentru A^2:
		cblas_dtrmm(101, // row-major
			    142, // matricea descrisa se afla pe partea dreapta
			    121, // matricea este upper-triangular
			    111, // nu avem nevoie de transpusa
			    131, // non-unit matrix 
			    N, N, // nr linii & coloane
			    1, // scalar
			    A, // matricea initiala A
			    N, // precizie A
			    A_squared, // in care se copiaza valorile din A inainte de apelarea functiei; nu se poate folosi tot A deoarece rezultatul returnat este incorect (valorile din A sunt suprascrise) 
			    N); // precizie A_squared

		3) pentru A^2 * B:
		cblas_dtrmm(101, // row-major
			    141, // matricea descrisa se afla pe partea stanga
			    121, // matricea este upper-triangular (upper_trian * upper_trian => upper_trian)
			    111, // nu avem nevoie de transpusa
			    131, // non-unit matrix 
			    N, N, // nr linii & coloane
			    1, // scalar
			    A_squared, // prima matrice
			    N, // precizie A_squared
			    B, // matricea in care se retine rezultatul 
			    N); // precizie B

	   - dupa apelarea functiilor am obtinut 2 matrici corespunzatoare celor 2 inmultiri, result si B;
	   - pentru finalizarea calculului am adunat valorile din B la valorile din result si am returnat result;
	   - rezultatele obtinute sunt corecte :)


## neopt:	
	   - am tinut cont de faptul ca A este upper triangular in calculul transpusei & in ridicarea la putere, unde am efectuat operatii numai cu valorile nenule;
	   - restul inmultirilor s-au facut clasic, trecand prin toate valorile;
	   - se putea tine cont si aici de triangularitate, definind 2 functii separate, care sa abordeze fiecare caz de inmultire:
		1) B * A_transpose, unde A_transpose devine lower triangular 
		2) A^2 * B, unde A^2 ramane upper triangular

	   - am refolosit functia copy_mat & mi-am definit cate o functie pentru fiecare operatie: 
		- get_transpose: intoarce transpusa unei matrice upper triangular date ca parametru
		- get_squared: intoarce patratul unei matrice upper triangular
		- multiply_matrices: realizeaza inmultirea a doua matrice
		- get_sum: realizeaza suma a doua matrice

	opt_m: 
	   - principalele modificari facute au fost folosirea registrilor si renuntarea la adresele vectoriale prin dereferentiere (ah, yes, pointers!)
	   1) get_squared:
		- am folosit un register double pentru a inlocui constanta res[i * N + j] din ciclul interior for (k = i; k <= j; ++k), la care se aduna valori ce tin de k; 
		- la iesirea din ciclu, res[i * N + j] primeste valoarea din registru;
	   2) multiply_matrices:
		- am folosit atat registrul din get_squared, cat si pointeri pentru parcurgerea mai rapida a matricelor;
		- pentru calculul adreselor pointerilor m-am folosit de exemplul din laboratorul 5, adaptat pentru matricea liniarizata

	   - in cazul neoptimizat, rularea cu N = 1200 se face in  32-35s, iar cu modificarile de mai sus ajungem la 18-20s
	   - rezultatele obtinute sunt corecte :)

	opt_f:
	   - cazul N = 1200 da timpi de ~7s

	opt_f_extra:
	   - cazul N = 1200 da timpi de ~6s
	   - flag-uri folosite:
		- "-ftree-loop-distribution": imbunatateste performanta cache-ului pentru cicluri mari si permite alte tipuri de optimizari (vectorizare & paralelizare)
		- "-floop-nest-optimize": optimizeaza ciclurile pentru paralelizare & acces mai rapid la date
		- "-ftree-loop-if-convert": special pentru vectorizarea matricelor, dispare control flow-ul de la inner loops si se acceseaza mai usor datele
		- "-ffast-math": seteaza mai multe flag-uri specifice operatiilor matematice, ce imbunatatesc viteza programului
		- "-funroll-all-loops": realizeaza loop unrolling, imbunatatind viteza prin reducerea iteratiilor


## Comparative analysis:
		- graficele se afla in folderul plots si sunt denumite dupa cazul abordat
		- datele folosite pentru reprezentare se afla in plots_data.txt
		- valori adaugate: N = 200 si N = 1600
		- graficele sunt realizate folosind timpii de rulare rezultati & indexul rularii
		- am ales pentru reprezentare cazurile de interes din tema:

		1) neopt vs opt_m, N = 1200
		- in plot se distinge clar diferenta intre timpii de rulare, varianta optimizata fiind cu ~14s mai rapida
		- avem un spike pentru cazul optimizat, la a doua rulare, posibil cauzat de valori cache-uite 

		2) opt_f vs opt_f_extra, N = 1200
		- rapiditatea opt_f_extra fata de opt_f este evidenta
		- timpii de rulare cresc/scad cu valori destul de mici, deoarece ambele metode folosesc -O3, iar flag-urile extra nu fac o diferenta extrem de mare dupa folosirea acestuia 

		3) all
		- am afisat datele obtinute pentru fiecare rulare a fiecarui program, cu fiecare N (200 -> 1600)
		- se observa clar cum timpii de rulare scad pe masura ce optimizam algoritmul, diferenta cea mai mica fiind intre opt_f si opt_f_extra
		- varianta blas este cea mai eficienta dintre toate, dupa cum era de asteptat, timpii fiind extrem de mici fata de restul, indiferent de N

		- am realizat grafice si pentru fiecare N dat, facand comparatie intre toate metodele de calcul (fisierele all_N.png)
		- ca si in celelalte figuri, se observa clar diferentele intre timpii de rulare, opt_f si opt_f_extra fiind cele mai apropiate 

