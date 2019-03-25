#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


//------------------------------------------------------------------------

/*CONVENTION: graphe non oriente. les aretes sont encodees dans un seul sens
donc on ajoutera le sens reciproque. */

/*Choix de design: le graphe est cree a partir du fichier txt et les personnes
sont numerote dans lordre classique de lecture du graphe grille
Le but et de generaliser ce code pour traiter tout type de graphe avec une 
application sur les graphes grille*/

typedef enum { Sain, Malade, Immunise, Mort} State;

/*etat de chaque noeud de depart a specifier dans le fichier et definit
a la creation du graphe -> malus: txt volumineux
OU 
on donne le nb de chaque personne dans chaque etat et le txt nous donne
seulement le numero des personnes non saines : methode adoptee*/

typedef struct _cell { int indexInList ; struct _cell* next; } Cell;

typedef struct { int nb_summit; Cell** successors; State* population_states; } Graphe;
//population_states = table des personnes

/*liste chainee pour connaitre les successor mais pour connaitre leur info
se refferer a une liste a part. les liste de successor permettent uniquement
de se refferer dans la liste a part.

letat est modofie seulement dans population_states et on y accede dans le parcours des
successeurs par lindexe de la presonne concernee*/

//------------------------------------------------------------------------

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
			printf("%d", G->population_states[index]);
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

void evolve_calculation(Graphe* G, int alpha, int beta, int gamma){
	
	double likelihood, global_alpha;
	Cell* current_successor;

	for(int i = 0; i < G->nb_summit; i++){
		//on lance les des
		likelihood = rand()%100;

		if 		(G->population_states[i] == Sain){
			global_alpha = 0;
			current_successor = G->successors[i];
			while(current_successor != NULL){
				if(G->population_states[current_successor->indexInList] == Malade)
					global_alpha = global_alpha + alpha;
				current_successor = current_successor->next;
			}
			if(likelihood <= global_alpha*100)
				G->population_states[i] = Malade;

		}else if(G->population_states[i] == Malade){
			if(likelihood <= beta)
				G->population_states[i] = Mort;
			else if(likelihood >= (1-gamma)*100)
				G->population_states[i] = Immunise;
		}
	}
}


//------------------------------------------------------------------------

int main(){


	int timestep = 100;
	double alpha = 0.4, beta = 0.3, gamma = 0.7;
	const char* grapheFileName = "test.txt";
	Graphe G;
	srand(time(NULL));

	/*plan: 
	-> lire un fichier qui donne les connexions et specifie letat initial
	-> construire le graphe
	-> simuler lavancer du temps:
		@ sain : proba de devenir malade alpha
		@ malade : proba de devenir imuniser gamma ou mort beta
	*/

	graphe_creation(&G, grapheFileName);
	//print_graphe_arcs(&G);

	while(timestep--){

		//affichage tous les 10 unites de temps:
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