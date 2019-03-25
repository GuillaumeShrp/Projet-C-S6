#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>

typedef enum { Sain, Malade, Immunise, Mort} State;
typedef struct _cell { int indexInList ; struct _cell* next; } Cell;
typedef struct { int nb_summit; Cell** successors; State* population_states; } Graphe;
//population_states = table des etats des personnes

/*liste chainee pour connaitre les successor mais pour connaitre leur info
se refferer a une liste a part. les liste de successor permettent uniquement
de se refferer dans la liste a part.

letat est modofie seulement dans population_states et on y accede dans le parcours des
successeurs par lindexe de la presonne concernee*/

void graphe_creation(Graphe* G, const char* grapheFileName);
void print_graphe_states(Graphe* G);
void print_graphe_arcs(Graphe* G);
void evolve_calculation(Graphe* G, double alpha, double beta, double gamma);