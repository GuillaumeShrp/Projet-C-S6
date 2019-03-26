#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "code.h"


//------------------------------------------------------------------------

/*CONVENTION: graphe non oriente. les aretes sont encodees dans un seul sens
donc on ajoutera le sens reciproque. */

/*Choix de design: le graphe est cree a partir du fichier txt et les personnes
sont numerote dans lordre classique de lecture du graphe grille
Le but et de generaliser ce code pour traiter tout type de graphe avec une 
application sur les graphes grille*/

/*etat de chaque noeud de depart a specifier dans le fichier et definit
a la creation du graphe -> malus: txt volumineux
OU 
on donne le nb de chaque personne dans chaque etat et le txt nous donne
seulement le numero des personnes non saines : methode adoptee*/

/*INFO:
le nombre daretes dun graphe grille de n^2 personnes est de (n-1).n.2*/

//------------------------------------------------------------------------

void graphe_creation(Graphe* G, const char* grapheFileName){
	
	FILE *fp;
	fp = fopen(grapheFileName, "r");
	
	if (fp != NULL){
		int nb_summit, nb_ill, nb_arc, index_from, index_to, index_ill;

		//initialisation du graphe:
		fscanf(fp, "%d%d%d", &nb_summit, &nb_ill, &nb_arc);	
		G->nb_summit = nb_summit;
		G->successors = (Cell**)malloc(nb_summit * sizeof(Cell*));
		G->population_states = (State*)malloc(nb_summit*sizeof(State));

		for (int i = 0; i < nb_summit ; i++){
			G->population_states[i] = Sain;
			G->successors[i] = NULL;
		}

		for (int i = 0; i < nb_ill ; i++){
			fscanf(fp, "%d", &index_ill);
			G->population_states[index_ill] = Malade;
		}

		for (int i = 0; i < nb_arc; i++){

			fscanf(fp, "%d %d", &index_from, &index_to);

			Cell* new_successor_direct = (Cell*) malloc(sizeof(Cell));
			new_successor_direct->next = G->successors[index_from];
			new_successor_direct->indexInList = index_to;
			G->successors[index_from] = new_successor_direct;

			//sens reciproque (graphe non oriente):
			Cell* new_successor_reciproque = (Cell*) malloc(sizeof(Cell));
			new_successor_reciproque->next = G->successors[index_to];
			new_successor_reciproque->indexInList = index_from;
			G->successors[index_to] = new_successor_reciproque;
		}
	}

	else printf("Le fichier n'a pas été trouvé.");
	fclose(fp);
}

void print_graphe_states(Graphe* G){
/*fonction daffichage valable uniquement pour un graphe grille*/
	int index = 0, nb_ligne = (int)pow(G->nb_summit, 0.5);

	while(index != G->nb_summit){
		for (int j = 0; j < nb_ligne ; j++){

			//les personnes mortes ne sont pas afficher:
			if(G->population_states[index] == 3)
				printf(" ");

			else printf("%d", G->population_states[index]);
			index++;
		}
		printf("\n");
	}
}

void print_graphe_arcs(Graphe* G){
	printf("liste d'adjacence:\n");
	Cell* current_successor;
	for(int i = 0; i < G->nb_summit; i++){
		printf("n° %d:", i);
		current_successor = G->successors[i];
		while(current_successor != NULL){
			printf(" %d", current_successor->indexInList);
			current_successor = current_successor->next;
		}
		printf("\n");
	}
}

void evolve_calculation(Graphe* G, double alpha, double beta, double gamma){
	
	double likelihood, global_alpha;
	Cell* current_successor;

	/*PROBLEME: les calculs sont dynamiques pour un meme temps t
	ie les evolutions calucle pour t+1 sont prise en compte dans les
	calculs des sommets au temps t
	SOLUTION: enregister les etats de t+1 calcul dans une structure a 
	part pour ne pas modifier G->successors en court de calcul*/

	State population_next_states[G->nb_summit];

	for(int i = 0; i < G->nb_summit; i++){
		//on lance les des:
		likelihood = rand()%100;

		if 		(G->population_states[i] == Sain){
			global_alpha = 0;
			current_successor = G->successors[i];
			while(current_successor != NULL){
				if(G->population_states[current_successor->indexInList] == Malade)
					global_alpha = global_alpha + alpha;
				current_successor = current_successor->next;
			}
			if(likelihood < global_alpha*100)
				/*Attention: inegalité stricte sinon likelihood peut etre nulle 
				et alors un malade apparait de lui meme et non a cause de ses voisins*/
				population_next_states[i] = Malade;
			else population_next_states[i] = Sain;

		}else if(G->population_states[i] == Malade){
			//meme attention pour linegalite stricte (du cote de 0 uniquement):
			if(likelihood < beta*100)
				population_next_states[i] = Mort;
			else if(likelihood > (1-gamma)*100)
				population_next_states[i] = Immunise;
			else population_next_states[i] = Malade;

		}else population_next_states[i] = G->population_states[i];
	}

	//actualisation des etats de la population suivant les calculs devolution:
	for(int i = 0; i < G->nb_summit; i++)
		G->population_states[i] = population_next_states[i];
}

void creation_txt_gird(int n,const char* createdGrapheFileName){
//n = taille de grille

    FILE *fp;

    fp = fopen(createdGrapheFileName,"w");
    //nombre darete = n*(n-1)*2
    fprintf(fp,"%d\n%d\n%d\n%d\n", n*n, 1, n*(n-1)*2, n*n/2-1-n/2);
	/*creation dun graphe grille avec un malade au milieu
	soit a lindex n*n/2-1-n/2 */

    //creation des arretes lignes:
    for (int i = 0; i < n; i++)
    	for (int j = 0; j < n-1; j++)
    		fprintf(fp, "%d %d\n", i*n +j, i*n +j+1);
    //creation des arretes colones:
    for (int i = 0; i < n-1; i++)
    	for (int j = 0; j < n; j++)
    		fprintf(fp, "%d %d\n", i*n +j, i*n +j+n);

    fclose(fp);
}

//------------------------------------------------------------------------

int main(){

	int timestep = 20;
	double alpha = 0.2, beta = 0.1, gamma = 0.05;
	const char* grapheFileName = "testy.txt";
	Graphe G;
	srand(time(NULL));

	/*plan: 
	-> lire un fichier qui donne les connexions et specifie letat initial
	-> construire le graphe
	-> simuler lavancer du temps:
		@ sain : proba de devenir malade alpha
		@ malade : proba de devenir imuniser gamma ou mort beta
	*/

	//premiers tests:
	//graphe_creation(&G, "test.txt");
	//print_graphe_arcs(&G);
	
	creation_txt_gird(20,grapheFileName);
	graphe_creation(&G, grapheFileName);

	while(timestep--){
		//affichage partiel de levolution:
		if(timestep % 10 == 0){
			print_graphe_states(&G);
			printf("\n");
		}
		evolve_calculation(&G, alpha, beta, gamma);
	}


	return 0;
}


/*PISTES:
-> automatiser la creation du fichier dencodage du graphe .. cf:Theophile
-> djisktra : apparition dun vaccin chez une personne et propagation
au malade le plus proche 
*/
