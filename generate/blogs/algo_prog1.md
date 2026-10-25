# Formation d'AlgoProg 1
Ce blog est issu d'une formation que j'ai mise au point pour le cours d'*Algorithmique et Programmation 1* de 1ère année à l'ISMIN (au moment où j'écris ce blog, j'y suis en 3ème année). L'objectif est de transmettre les concepts fondamentaux de la programmation en C (tels que la *synthaxe* du langage ou l'utilisation des *pointeurs*). Le publique ciblé est donc principalement les personnes n'ayant jamais programmé en C, et qui, de manière générale, n'ont que très peu codé dans leur vie (au plus un peu de *Python* pour les cours).

## Sommaire
1. Qu'est-ce que C ?
1. Synthaxe et structure du langage
1. Manipulations de la mémoire en C
1. Chaînes de charactères
1. Manipulation de flux (fichiers et entrées/sorties du terminal)
1. Structures de base: *listes chaînées*, *piles*/*files* et *tas binaires*
1. Compilation et modularité

# 1. Qu'est-ce que C ?
Le **C** est un langage de programmation:
- ***compilé*** : le code source doit d'abord être converti en code machine avant de pouvoir être exécuté par l'ordinateur.
- ***bas niveau*** : dans le sens "qui est à peine plus agréable à utiliser que du binaire"
- ***impératif*** : chaque instruction s'exécute dans l'ordre où elle apparait dans le code source.
- ***fortement typé*** : chaque variable et fonction possède un unique *type* et on ne peut pas directement passer d'un type à un autre sans expliciter ce que l'on fait. Ce type représente la structure en mémoire de la variable.
- ***vieux*** : il y a plusieurs aspects du langage qui semblent très arbitraires ou abstraits aujourd'hui mais qui proviennent des limitations à l'époque de sa concéption (ça date des années 70 quand même).

Parmis certaines particularités à garder en tête (on définira tous les termes plus tard) on a :
- La fonction `void main() { ... }` ou `int main() { ... }` doit **nécessairement être définie** et est la première fonction qui sera appelée à l'exécution du code (on dit que c'est le *point d'entrée du programme*).
- Toutes fonctions, toutes variables et tous types doivent être déclarés **avant** d'être utilisés ou définis (les deux peuvent se faire en même temps).
- Un code compilé sur un ordinateur ne va pas forcément fonctionner sur un autre, il est donc préférable de compiler le code sur la nouvelle machine.

# 2. Synthaxe et structure du langage
## 2.1 Premier programme
Voici un exemple basique d'un programme en C pour illustrer sa synthaxe :
```C
// Ceci est un commentaire sur une ligne
/*
    Celui-ci est sur toute une zone balisée
*/

/* Equivalent en Python à "import module", ça nous permet de faire appel à du code existant en
dehors de notre fichier. En l'occurence, on a besoin de la fonction `printf` */
#include <stdio.h> 

// Déclaration d'une fonction :
    void print_salutation(char *name, int time_of_day)
//  ^    ^                ^
//  |    |                Liste d'arguments que prend la fonction (ici une chaîne de caractères 
//  |    |                "name" et un entier "time_of_day")
//  |    Nom de la fonction
//  Type de la valeur renvoyée (ici `void` car elle ne renvoie rien)
{
    // Sections conditionnelles :
    if (time_of_day == 0)
    {
        // Appel de fonction :
        printf("Good morning, %s!\n", name);
    //  ^      ^
    //  |      Liste de paramètres
    //  Nom de la fonction
    }
    else if (time_of_day == 1)
    {
        printf("Good evening, %s!\n", name);
    }
    else // Si aucune des conditions avant n'a étée vérifiée, on éxecute cette branche
    {
        printf("Good night, %s!\n", name);
    }
}

// Point d'entrée du programme (aka. la fonction qui sera appelée à l'exécution du programme 
// compilé)
int main()
{
    print_salutation("Vlad", 1); // Attendu: "Good evening, Vlad!\n"
    return 0;
}
``` 
C'est un exemple non-exhaustif, mais il couvre les catégories fondamentales qui sont les ***commandes préprocesseurs*** (qui commencent par `#`), les ***fonctions*** (comme `print_salutation`), le ***flow de contrôle*** (qui introduisent plusieurs chemins d'éxecution du code, par exemple `if ... else ...`) et les ***expression*** (globalement tout ce qui fait intervenir un opérateur arithmétique, comme `==`).
Enfin, et c'est la principale source d'erreurs de compilation chez les débutants, il faut finir toute ses lignes de code par un `;` (ce n'est pas toujours nécessaire mais vaut mieux en mettre trop que pas assez).

> ### /!\ LA FONCTION MAIN DOIT OBLIGATOIREMENT APPARAÎTRE DANS LE CODE CAR C'EST LA PREMIÈRE FONCTION QUI SERA APPELÉE LORS DU LANCEMENT DU PROGRAMME /!\

## 2.2 Les différents énoncés de flow de contrôle
Un *énoncé de flow de contrôle* (j'ai pas trouvé de meilleur nom) permet de modifier la trame d'exécution du code (qui est par défaut parfaitement séquencielle et uni-directionnelle). Souvent, une condition entre en jeu pour décider ou non du branchement du code.  
Voici une liste de ceux qui vous seront utiles :

```C
// Scope : section de code encardée par deux accolades. Elle permet de compartimenter le code 
// (nottament pour les déclarations de variables).
{
    ...
}

// If-else : Si `condition1` est respectée, executer la scope.
//           Sinon, si `consition2` est respectée, executer la scope.
//           ...
//           Sinon, executer la scope
if      (condition1) { ... }
else if (condition2) { ... }
else if (condition3) { ... }
...
else { ... }
// Switch-case : Si `expression` vaut `valeur1`, executer la scope
//               Sinon,  si elle vaut `valeur2`, executer la scope
//               ...
//               Sinon, executer la scope 
//               /!\ LES VALEURS DES `CASE` DOIVENT ÊTRE DES CONSTANTES TOUTES DIFFÉRENTES /!\ 
switch (expression) {
    case valeur1: { ... } break;
    case valeur2: { ... } break;
    ...
    default:      { ... } break;
} 

// Boucle `while` : tant que `condition` est respectée, exécuter la scope 
while (condition) { ... }
// Boucle `for` : pour un entier `i` commençant à 0, tant qu'il est inférieur à N, executer la 
// scope puis incrémenter `i`
for (int i = 0; i < N; i++) { ... }

// Sortie de fonction : permet de quitter la fonction en cours d'exécution et de renvoyée la 
// valeur `expression` (ou rien si la fonction ne renvoie pas de valeur)
return expression;
// Sortie de boucle : permet de quitter la boucle en cours d'exécution (que ce soit `while` ou 
// `for`) prématurément ou de quitter le `switch` courant. 
//                    /!\ NE S'APPLIQUE QUE DANS DES BOUCLES OU DANS UN SWITCH-CASE /!\ 
break;
```

> **(i)** La synthaxe du `switch-case` est très étrange à premier abord car elle provient à l'origine d'une optimisation pour du code en *assembleur* dans lequel il n'y a pas de fonctions ou d'énoncés de flow de contrôles (comme dans les langages plus *haut niveau*) mais uniquement des `label` (nom donné à une position précise dans le code) et des instructions de `jump` conditionnels ou non (`jump` permet notamment de passer à l'execution séquencielle depuis un `label` si une condition est respectée).

> **(i)** La synthaxe de la boucle `for` n'impose pas de compter jusqu'à `N` avec un entier qui incrémente à chaque itération, on peut faire bien plus (cf. 6.1 pour un exemple de parcours de liste chaînée utilisant une boucle `for`). 

## 2.3 Utiliser une variable
Comme évoqué précédement, le C est un langage fortement typé ce qui impose donc aux variables d'avoir un type explicitement défini :
```C
// Déclaration d'une variable `a` de type `entier signé`
int a;
a = 1;
// On peut d'ailleurs fusionner ces deux lignes :
int a = 1;

// On ne peut cependant pas juste écrire :
a = 1; // [ERROR] Undeclared identifier "a" (la variable a n'a jamais été déclarée, on ne peut pas
       // lui assigner de valeur)

// Et il faut éviter de lire depuis une variable déclarée mais pas initialisée :
int a;
int b = a; // Quelle est la valeur de `b` ? eh... ça dépend des fois
```

Les variables déclarées au sein d'une *scope* (section de code entre accolades) sont *locales* à celle-ci, dans le sens où elles ne sont plus accessibles après fermeture de leur scope parent :
```C
int variable_outside_scope = 42;

// Et voici la fameuse scope
{
    // Cette variable est locale à la scope. On peut accéder aux variables déclarées dehors de la 
    // scope depuis l'intérieur
    int variable_inside_scope = variable_outside_scope;
}

// La variable `variable_outside_scope` ayant été déclarée en dehors de la scope, elle existe
// encore ici et on peut donc l'utiliser sans soucis
variable_outside_scope = variable_outside_scope + 1;

// Par contre, `variable_inside_scope` n'existe plus car l'accolade s'est refermée -> Le code ne
// compile pas
if (variable_inside_scope == 42) // [ERROR] Undeclared identifier "variable_inside_scope"
{
    printf("The variable inside of the scope has the value 42!\n");
}
```

> ### /!\ ATTENTION ! CECI EST TRÈS DIFFÉRENT DU PYTHON ! IL VA FALLOIR SE FAMILIARISER AVEC CE "FORT TYPAGE" ET CETTE "LOCALITÉ" DES VARIABLES /!\

## 2.4 Les types
Le C possède un certain nombre de types "basiques" et permet aussi de créer des types dits "composites" ou "structurés". Ceux-ci sont utilisés pour expliciter la représentation en mémoire des variables dans le code (par exemple, pour donner la taille en octets qu'occupe la variable).

### 2.4.1 Les types de base
Le C propose plusieurs types fondamentaux :
- `int` : entier signé (sur 32-bits). Il peut représenter des valeurs dans **[-2³¹ ; 2³¹-1]**.
- `unsigned int` : entier non-signé (sur 32-bits). Il peut représenter des valeurs dans **[0 ; 2³²-1]**.
- `float` : réel (sur 32-bits).
- `char` : caractère ASCII (sur 8-bits). Fondamentalement, c'est un entier sur 8-bits, mais il est très souvent utilisé avec les chaînes de caractères.
- `bool` : booléan (sur 8-bits). Peut prendre la valeur `true` (1) ou `false` (0).
- `void` : rien (sur 0-bits). Ce type n'héxiste que pour permettre aux fonctions de ne pas avoir à forcément renvoyer une valeur.

La liste précédente est non-exhaustive, elle correspond aux types les plus fréquents et qui peuvent vous être imposés en exam.


> **(i)** De plus, le type `size_t` apparait très souvent dans les définitions de fonctions de la librairie standard. Celui-ci correspond à un entier non-signé assez large.

> ### /!\ ATTENTION ! LE TYPE `bool` N'EST PAS DÉFINIT PAR DÉFAUT ET DOIT ÊTRE INCLU AVEC `#include <stdbool.h>`. CECI EST AUSSI VRAI POUR LES CONSTANTES `true` (1) et `false` (0) /!\

### 2.4.2 Les types structurés (*struct*)
Pour pouvoir représenter des données plus complexes que de simples nombres, on peut être amenés à devoir utiliser des données structurées, ou *struct* :
```C
// Déclaration de la structure `struct MyStruct` composée de 3 variables : un entier `x`,
// un réel `y` et un caractère `c`.
struct MyStruct
{
    int   x;
    float y;
    char  c;
};
// Déclaration d'une variable de type `struct MyStruct`.
struct MyStruct variable;
// Et non pas :
MyStruct variable; // [ERROR]: Unknown identifer "MyStruct" (le nom du type structuré est
                   // `struct MyStruct` et pas seulement `MyStruct`).

// Déclaration du type `MyStruct` comme un alias de `struct MyStruct`
typedef struct MyStruct MyStructType;
//      ~~~~~~~~~~~~~~~ ^
//      ^               Nom du type créé comme alias
//      Nom du type initial

// Ou alors, en une seule déclaration :
typedef struct MyStruct {
    int   x;
    float y;
    char  c;
} MyStructType;

// Assignation d'une valeur à notre variable de type structurée
MyStructType variable = (MyStructType){ .x = 42, .y = 0.987, .c = 'a' };
//                  ~~~~~~~~~~  ~~~~~~~
//                  ^           ^
//                  |           Assignation de la variable interne (ou champs) `x` du type `MyStruct`
//                  Nom entres parenthèses pour expliciter le type qu'on veut utiliser

// On peut alors assigner les champs individuelement :
variable.x = 67;
// Ou les lires :
float value_y = variable.y;
```

Il est à noter que dans l'exemple ci-dessus, le nom initial du type structuré est bien `struct MyStruct` et pas seulement `MyStruct` car dans ce contexte `MyStruct` n'est que le nom de la représentation structurelle et non du type qui en découle. Cette nuance peut porter à confusion, c'est pourquoi je vous recommande d'utiliser les `typedef` pour s'assurer de bien faire référence au type défini.

## 2.5 Les fonctions
Comme pour les variables, les fonctions en C sont typées. En particulier, les paramètres d'entrée de la fonction doivent avoir un type explicite et la valeur de retour (si la fonction renvoie quelque-chose) doit aussi être explicitée :
```C
// Déclaration d'une fonction :
    float fonction_quelconque(int arg1, float arg2, MyStructType arg3)
//  ~~~~~ ~~~~~~~~~~~~~~~~~~~ ~~~~~~~~  ~~~~~~~~~~  ~~~~~~~~~~~~~~~~~
//  ^     ^                   ^
//  |     |                   Liste d'arguments (déclarations de variables séparées par des virgules)
//  |     Nom de la fonction
//  Type de la valeur renvoyée ou `void`
{
    ...
}

// Cette fonction ne renvoie aucune valeur, elle a donc un type de retour `void`
void fonction_qui_ne_renvoie_rien(int a, int b)
{
    ...
}

// Cette fonction n'accepte aucun argument, il n'y a donc rien entre les parenthèses
char fonction_sans_arguments()
{
    ...
}



// Exemple de fonction : calculer le carré d'un entier
int puissance2(int n)
{
    return n * n;
}

// Appel de fonction : on passe des expression en argument, on capture la valeur renvoyée
int a = puissance2(2); // a = 4
```

Il est à noter que les paramètres de fonctions sont des copies des expression passées en entrée lors de l'appel ce qui implique donc qu'une fonction ne peut pas modifier ses paramètres d'entrée en dehors du contexte de la fonction. Ceci a deux conséquences :
- On ne peut pas modifier les valeurs originales en entrée, donc on peut modifier les valeurs des paramètres d'entrée sans avoir peur de modifier leurs valeurs originales. On peut alors considérer que les paramètres d'entrée d'une fonction sont déclarés dans la même *scope* que le reste de la définition de la fonction (ce qui est cohérent avec la synthaxe de déclaration de la liste de paramètres)
- On ne peut pas modifier les valeurs originales en entrée, donc il faut employer un autre mécanisme si on veut être capable de le faire : les pointeurs ***(cf. 3.1)***. En prenant des pointeurs en argument, une fonction peut alors modifier les données pointées sans soucis.

## 2.6 Les directives de préprocesseur *(qui commencent par `#`)*
Les directives de préprocesseur sont des instructions qui seront exécutées avant que le programme soit compilé : elles modifient directement le code source.  
Les deux plus importantes sont :
- #### `#include <...>` ou `#include "..."`
  Inclusion de code externe au sein du code courant. Ceci permet d'utiliser des librairies externes, comme par exemple que `stdio.h` qui permet entres-autres d'afficher du texte dans le terminal ou d'écrire dans un fichier.

- #### `#define {name} {definition}`
  Création d'une *macro* qui, à tout endroit où elle apparaît dans le code source, sera remplacée par sa définition. Cela permet par exemple de créer des constantes (exemple : `#define PI 3.141592`).

# 3. Manipulations de la mémoire en C
## 3.1 Démystification des *pointeurs*
Lorsqu'un programme s'exécute, il a très souvent besoin de modifier des variables et de conserver des valeurs pour les réutiliser plus tard : il a besoin d'une *mémoire*.  
Cette mémoire doit être imaginée comme une succession de cases pouvant chacune contenir 1 octet (ou 8 bits) arrangées en ligne. On peut alors identifier une case par sa position dans la ligne, ce qu'on appelle en informatique son *adresse* (comme une adresse de maison si la ligne était une route). Une adresse est donc un entier allant de 0 (la première boîte, qui est au tout début de la ligne et donc décalée de 0 par rapport au début de celle-ci) à N, la mémoire allouée au programme qui s'exécute. Cette adresse va être très importante en C car elle va permettre aux programmes de pouvoir lire et modifier des données à des endroits précis de la mémoire de façon *absolue*. Cette adresse peut alors être stockée dans une variable, que l'on appelera un *pointeur* (car elle indique la position d'un élément en mémoire, pas parce qu'elle touche des gosses) :

```C
    float f = 3.141592;
// Voici un exemple de déclaration de pointeur en C :
    float * p = & f;
//  ^     ^ ^   ^ ^
//  |     | |   | Variable contenant un float (3.141592)
//  |     | |   Opérateur renvoyant l'adresse mémoire de la variable `f`
//  |     | Nom de la variable contenant un entier correspondant à l'adresse en mémoire de f
//  |     Asterisc signifiant "pointeur vers un float"
//  Type de donnée vers laquelle pointe `p`

// Il est important de se rappeler que `p` n'est pas un `float` mais bien une adresse mémoire.
float a = p; // [ERROR]: Invalid converion between "float" and "float *"
// Cependant, on peut s'imaginer que "*p" est le nom d'une variable de type `float` :
float a = *p; // Parfaitement valide, `a` prendra la valeur vers laquelle pointe `p`, ici 3.141592
```
Voici une schématisation de la mémoire dans l'exemple ci-dessus :
```
.-------------------------------------------------------- - - -
|0.       |1.       |2.       |3.       |4.       |5.       |
|   ...   | f=3.141 |  420.0  |   ...   |   p=1   |   ...   
|         |         |         |         |         |         |
'---------+---------+---------+---------+---------+------ - - -
                A                            |
                '----------------------------'
                   p est un pointeur vers f
```

Maintenant considérons ceci :
```C
float *p2 = p + 1; // Avec `p` le pointeur du dernier exemple
float  f2 = *p2;   // Qu'est-ce qu'on vient de lire ?
```
Un pointeur n'étant qu'une variable entière, on peut utiliser des opérateurs arithémtiques dessus pour *décaler* l'adresse et ainsi lire une autre valeur. Dans l'exemple ci-dessus, on lit donc la valeur à l'adresse *2* de notre mémoire, donc `f2` vaut *420.0*.

Ainsi, on peut définir un tableau en *C* comme étant une succession continue de cases en mémoire :
```C
float tableau[] = { 1.0, 2.5, -4.0, 67.0 };
//           ~~   ~~~~~~~~~~~~~~~~~~~~~~~~
//           ^    ^
//           |    Chacune des valeurs entres accolades se suit en mémoire
//           Indique au compilateur qu'on créé un tableau dit "statique" (cf. 3.2.1)
// (i) La variable `tableau` est alors un pointeur vers le premier élément du tableau

// L'élément à l'indice 2 se trouve à l'adresse du tableau (aka. l'adresse
// du premier élément du tableau) décalé de 2 vers la droite :
float element_indice_2 = *(tableau + 2);
// Ce qui s'écrit plus succintement en C :
float element_indice_2 = tableau[2];
```

> ### /!\ ATTENTION ! IL EST TRÈS FACILE DE FAIRE UN CALCUL DE POINTEUR ERRONÉ OU DE LIRE À UN INDICE DE TABLEAU TROP GRAND ET DONC D'ESSAYER D'ACCÉDER À UNE CASE MÉMOIRE QUI N'A PAS ÉTÉ RÉSERVÉE PAR LE PROGRAMME ! LIRE DANS UNE CASE NON-RÉSERVÉE RÉSULTE EN L'ERREUR ***SEGMENTATION FAULT***. /!\

Enfin, lorsqu'on manipule des pointeurs vers des structures, on utilise la notation avec les flèches :
```C
typedef struct test_structure
{
    int a;
    float b;
} test_structure

test_structure *ts = ...;

// Pour accéder à `a`, on utilise l'opérateur de la flèche :
int valeur_dans_a = ts->a;
// Ce qui revient à lire la valeur dans la structure pointée par `ts` :
int valeur_dans_a = (*ts).a;
//                   ^   ^
//                   |   Lecture classique dans la structure
//                   Dérécférencement du pointeur `ts` pour travailler directement avec la structure
//                   pointée.
```


## 3.2 Division de la mémoire : la pile et le tas, et quand utiliser `malloc` et `free`
### 3.2.1 La pile

Lorsqu'une fonction est appelée, elle a besoin de réserver de la mémoire pour y stocker ses variables internes et deux-trois autres bricoles. En effet, nous avons vu avant que les variables sont *locales* à une scope synthaxiquement, mais il est aussi vrai qu'elles sont *locales* à une fonction, que ce soit synthaxiquement mais aussi en termes de mémoire. De ce fait, lorsqu'une fonction fini de s'exécuter et qu'elle n'a donc plus besoin de la mémoire qu'elle avait réservée, elle va la libérer ce qui, en conséquence, va détruire les variables internes à la fonction : les variables en C sont *éphémères* et n'exitent qu'à l'échelle d'une fonction.

La zone de la mémoire dans laquelle réservent les fonctions s'appelle la ***pile*** (ou *stack* en anglais), et elle est réservée au lancement du programme. Si on se visualise la pile comme verticale avec l'adresse 0 en bas, alors chaque à nouvel appel de fonction ses données réservées s'empilent au dessus des données de l'appel précedent : c'est ce qu'on appelle la ***pile d'appel*** (ou *call stack* en anglais). 

De plus, comme la pile est de taille fixe déterminée au lancement du programme, il se peut que trop de fonctions soient appelées sans finir (appels "en cascade") et que donc leurs données réservées *saturent* la pile. Alors, si une nouvelle fonction venait à être appelée, celle-ci n'aurait plus assez de place pour ses variables : c'est ce qu'on appel un ***stack overflow***, ou *débordement de pile* en français.

> **(i)** Un *stack overflow* arrive souvent lors d'appels récursifs si la condition d'arrêt n'est jamais atteinte.

### 3.2.2 Le tas
La pile nous permet déjà d'accomplir tout un tas de fonctionnalités et sera suffisante dans la majorité des cas, mais il arrive parfois qu'un programme ait besoin de réserver une zone de mémoire qui perdure après la fin de l'appel d'une fonction. Dans ce cas, il réserve une zone mémoire dans le ***tas*** (*heap* en anglais), une zone de la mémoire qui n'est associée à aucune fonction et qui n'est donc pas éphémère. Grâce à cette mémoire plus durable, on va être capable de manipuler des données dont la taille n'est pas forcément connue à la compilation (exemple : un tableau d'entiers dont la taille dépend d'un calcul) ou qui ne doivent pas être limitées au contexte d'une fonction (exemple : listes chaînées, cf. 6.1).  

**Cependant, c'est maintenant la responsabilité du programmeur de libérer cette mémoire une fois qu'elle n'est plus utilisée, contrairement au variables éphémères qui sont libérées automatiquement à la complétion d'une fonction.**
En effet, si la mémoire est allouée mais jamais libérée, on arrive au mếme problème que lors d'un stack overflow : le tas va se saturer et ne pourra plus réserver de nouvelle zone.

En C, le tas est accessible depuis plusieurs fonctions de la librairie standard `<stdlib.h>`, dont principalement `malloc` *(Memory ALLOCate, réserve de la mémoire)* et `free` *(libère de la mémoire précédemment réservée avec `malloc`)* :
```C
#include <stdlib.h>

float *allocate_float()
{
    float *f = malloc(sizeof(float));
//             ^      ~~~~~~~~~~~~~
//             |      ^      
//             |      Opérateur qui renvoie la taille en octets d'un type ou d'une donnée (ici, 
//             |      d'un float sur 32-bits soit 4-octets)
//             Fonction qui réserve `n` octets de mémoire dans le tas et renvoie un pointeur vers 
//             cette zone nouvellement allouée

    if (f == NULL) // La fonction malloc peut échouer, par exemple si il n'y a plus assez de 
                   // mémoire disponible sur l'ordinateur. Il faut donc vérifier que le pointeur 
                   // renvoyé est valide (aka. pas `NULL`).
    {
        printf("[ERROR]: insufficient memory!\n");
        return NULL;
    }

    *f = 0.0; // Pas nécessaire, mais c'est préférable d'initialiser la valeur renvoyée par 
              // `malloc` car elle contient des données arbitraires.
    return f;
}

int main()
{
    float *f = allocate_float();

    ...
    
    free(f); // Une fois qu'on a fini d'utiliser `f`, on libère la mémoire associée pour qu'elle 
             // puisse être réallouée plus tard
}
```

# 4. Chaînes de charactères
En C, les chaînes de charactères (ou *string* en anglais) sont des tableaux de charactères (`char`) se finissant pas un charactère nul `'\0'` (en anglais, elles sont décrites comme *null-terminated*). Lorsqu'on déclare une chaîne de charactère à l'aide de guillements, le compilateur ajoute automatiquement ce charactère nul à la fin de la chaîne :

```C
char hello[] = "Hello!";
printf("sizeof(hello) = %d\n", sizeof(hello)); // EXPECT: sizeof(hello) = 7
```
```
,-------------------------------------------------------.
|0.     |1.     |2.     |3.     |4.     |5.     |6.     |
|  'H'  |  'e'  |  'l'  |  'l'  |  'o'  |  '!'  |   0   |
|       |       |       |       |       |       |       |
'-------------------------------------------------------'
```

Il est important de garder ce charactère nul en tête lorsqu'on alloue de la mémoire à une chaîne de charactères car toutes les fonctions qui manipulent des chaînes de charactères s'attendent à ce qu'il soit présent pour connaître la taille de la chaîne :

```C
// Voici un exemple d'implementation de la fonction `strlen` de la librarie standard qui renvoie 
// la longeur d'une chaîne de charactères pour mettre en évidence l'importance du charactère nul
int strlen(char *string)
{
    int taille = 0;
    while (string[taille] != '\0')
    {
        taille ++;
    }

    return taille;
}
```

Plusieurs fonctions de manipulation de chaînes de charactères sont définies dans `<string.h>` :
- `int strlen(char *str)` : renvoie la longeur de la chaîne de charactères
- `strcpy(char *destination, char *source)` : copie la chaîne `source` dans la chaîne `destination`.
- `int strcmp(char *str1, char *str2)` : compare deux chaînes de charactères dans l'ordre lexicographique. Si la sortie vaut 0, les deux chaînes sont identiques.
- `sprintf(char *destination, char *format, ...)` : impression d'un texte formaté dans une chaîne de charactères (cf. 5.3).

> ### /!\ ATTENTION ! UNE CHAÎNE DE CHARACTÈRES EST BIEN DÉFINIE AVEC DES GUILLEMENTS ! L'UTILISATION D'APOSTROPHES EST RÉSERVÉE AUX CHARACTÈRES (EXEMPLE: `'a'` EST UN UNIQUE CHARACTÈRE, `"a"` EST UNE CHAÎNE DE 1 CHARACTÈRE) /!\

# 5. Manipulation de flux (fichiers et entrées/sorties du terminal)
Il arrive parfois qu'on ait besoin de manipuler une donnée externe au programme, comme un fichier, le terminal ou d'autres joyeusetés qu'on ne va pas aborder. Alors, on peut définir un ***flux*** comme une interface abstraite pour lire/écrire des données depuis la structure soujacente, que ce soit un fichier ou le terminal.

En C, la manipulation de flux se fait principalement à l'aide de la librairie standard `<stdio.h>` qui définit le type `FILE *` (notre flux) ainsi qu'une multitudes de fonctions agissant sur ce type.

## 5.1 Lecture et écriture dans un flux
Les fonctions de manipulation de flux sont toutes préfixées par `f` pour *File*.   
On retrouve pour l'écriture :
- `fwrite(void *pointer_to_values, size_t sizeof_value, size_t number_of_values, FILE *file)` : écriture d'une ou plusieurs donnée(s) arbitraire(s) dans un fichier.
- `fputc(int character, FILE *file)` : écriture d'un unique charactère dans un fichier.
- `fputs(char *string, FILE *file)` : écriture d'une chaîne de charactères dans un fichier.
- `fprintf(FILE *file, char *format, ...)` : écriture d'un texte formaté dans un fichier (cf. 5.3).  

Et pour la lecture :
- `fread(void *pointer_to_values, size_t sizeof_value, size_t number_of_values, FILE *file)` : lecture d'une ou plusieurs donnée(s) arbitraire(s) depuis un fichier.
- `fgetc(int character, FILE *file)` : lecture d'un unique charactère depuis un fichier.
- `fgets(char *string, FILE *file)` : lecture d'une chaîne de charactères depuis un fichier. Cette chaîne se termine par `\n` ou la fin du fichier.
- `fscanf(FILE *file, char *format, ...)` : analyse d'un texte formaté depuis un fichier (cf. 5.3).

> **(i)** Les entrées et sorties du terminal sont elles-aussi des flux, et sont nommés `stdin` *(STanDard INput, entrée textuelle de l'utilisateur)* et `stdout` *(STanDard OUtput, texte affichée sur le terminal)*, on pourrait donc utiliser les fonctions préfixés par `f`. Cependant, la librairie standard propose aussi des fonctions d'écritures de fichiers qui sont spécifiquement dirigées vers le terminal (celles-ci n'ont alors pas le préfixe `f` et sont donc les fameux `printf` et `scanf`).

## 5.2 Manipulation d'un fichier
Pour ouvrir un flux vers un fichier, on va utiliser la fonction `fopen` *(File OPEN)*. Celle-ci attend un chemin vers le fichier ainsi qu'un *mode d'ouverture* :
- `"r"` : ouvre le fichier en mode *lecture* (ou *read* en anglais). 
- `"w"` : ouvre le fichier en mode *écriture* (ou *write* en anglais).
- `"a"` : ouvre le fichier en mode *joindre* (ou *append* en anglais). Similaire au mode écriture, mais ne vide pas le fichier avant d'écrire.

De plus, si les données stockées dans le fichier ne sont pas textuelles, il peut être préférable d'ouvrir le fichier en mode *binaire* en ajoutant un `b` au mode d'ouverture (`"rb"`, `"wb"`, `"ab"`).

On peut ensuite fermer le flux en utilisant `fclose` :
```C
#include <stdio.h>

int main()
{
    FILE *fichier = fopen("chemin/vers/un/fichier.txt", "a");

    if (fichier == NULL)
    {
        printf("[ERROR]: couldn't open file!\n");
        return 1;
    }

    fputs("Ce texte a été ajouté à la fin du fichier!\n", fichier);

    fclose(fichier);
    
    return 0;
}
```

## 5.3 Texte formaté: `printf` et `scanf`
### 5.3.1 Impression de texte formaté avec `printf`
Lorsqu'on veut insérer un entier, un réel, un charactère ou une chaîne de charactère dans un flux ou une chaîne de charactères, on fait appel à la fonction `printf` *(PRINT Formated)* (ou `fprintf` pour les flux et `sprintf` pour les chaînes de charactères, les trois ayant la même synthaxe). La synthaxe est la suivante :
```C
printf("text %x %y", x, y);
//      ~~~~ ~~      ~~~~
//      ^    ^       ^
//      |    |       Liste des parametres à formater
//      |    Balise indiquant une expression à insérer dans la chaîne de charactères
//      Chaîne de charactère servant de "format"
```

Les balises de formatage (qui commencent par `%`) les plus courantes sont listées ci-dessous :
```C
printf(
    "An int: %d, a float: %f, a char: %c (or its ASCII code: %d), a string: \"%s\"\n", 
              1,         2.0,        'x',                   'x',   "a string"
);
```

Pour être plus précis, on peut ajouter des modificateurs (autres lettres, nombres ou symboles) entre le `%` et le `d`, `f`, `c` ou `s` :
```C
printf("int: %6d\n", 2);                    // Expect: "int:       2\n"
printf("int: %06d\n", 2);                   // Expect: "int: 0000002\n"
printf("float: %3.3d\n", 12.67192);         // Expect: "float:  12.672\n"
printf("float: %03.3d\n", 12.67192);        // Expect: "float: 012.672\n"
printf("string: %.8s\n", "plus-de-8-char"); // Expect: "string: plus-de-\n"
printf("string: %.8s\n", "moins");          // Expect: "string: moins\n"
printf("string: %8s\n", "plus-de-8-char");  // Expect: "string: plus-de-8-char\n"
printf("string: %8s\n", "moins");           // Expect: "string:    moins\n"
```

Bien évidement, il existe bien plus de balises et de modificateurs. 

### 5.3.2 Extraction d'expressions avec `scanf`
Lorsqu'on veut extraire des entiers, des réels, des charactères ou des chaînes de charactères depuis un flux ou une chaîne de charactères, on peut utiliser les fonctions de la famille `scanf` *(SCAN Formated)* (avec `fscanf` pour les flux et `sscanf` pour les chaînes de charactères). La synthaxe est identique à celle de `printf` mais, comme on veut lire des valeurs, on passe en argument des pointeurs vers des variables où stocker les données extraites :
```C
int a;
float f;
char c;
char str[25]; // On veut lire 24 charactères, donc +1 pour le charactère nul
scanf("int: %d, float: %f, char: %c, str: %.24s", &a, &f, &c, str);
// On écrit "int: 2, float: 12.67, char: z, str: chaine de 24 charactères\n"
// On s'attend à avoir a = 2, f = 12.67, c = `c`, str = "chaine de 24 charactères"
```

# 6. Structures de base
## 6.1 Liste chaînée
La liste chaînée est un ensemble de *maillons* qui pointent vers leurs successeurs. Ainsi, une liste chaînée se différencie d'un tableau par sa non-linéarité (chaque élément de la liste n'est pas focément à côté en mémoire) et par l'individualité de ses éléments (un noeud peut être alloué ou libéré de façon individuelle).

Voici un exemple de structure de liste chaînée :
```C
// La structure du maillon de la liste chaînée. Ici, c'est une liste d'entiers
typedef struct maillon maillon;
struct maillon
{
    maillon *suivant; // Pointeur vers le maillon suivant (ou `NULL` si c'est le dernier maillon)
    int valeur; // Valeur stockée dans ce maillon
};

// Structure de la liste chaînée (qui ne comporte dans cet exemple qu'un pointeur vers le premier
// élément mais qui pourrait aussi comporter la taille de la liste par exemple)
typedef struct liste
{
    maillon *tete; 
} liste;

int main()
{
    liste l = ...; // Imaginons que nous initialisons la liste avec des valeurs

    printf("liste l = [\n");

    // On parcours chaque noeud de la liste :
    maillon *m = l.tete; // On commence par la tete de liste
    while (m != NULL) // Tant qu'on est pas sur un maillon vide, on continue
    {
        printf("    %d\n", m->valeur);
        m = m->suivant; // On passe au maillon suivant. Si on était sur le dernier maillon de 
                        // la liste, alors `m->suivant` sera `NULL` et la boucle va s'arrêter
    }
    printf("]\n");

    // Ou, plus succintement :
    for (maillon *m = l.tete; m != NULL; m = m->suivant)
//       ~~~~~~~~~~~~~~~~~~~  ~~~~~~~~~  ~~~~~~~~~~~~~~
//       ^                    ^          ^
//       |                    |          Équivalent de `i++` dans un parcours de tableau, on passe
//       |                    |          au maillon suivant
//       |                    Équivalent de `i < MAX` dans un parcours de tableau, tant que le 
//       |                    maillon étudié n'est pas `NULL` (aka. qu'on a pas déjà parcouru le 
//       |                    dernier maillon)
//       Équivalent à `int i = 0` dans un parcours de tableau, on initialise au premier maillon de
//       la liste l
    {
        printf("    %d\n", m->valeur);
    }
    printf("]\n");

    return 0;
}
```

Voici la représentation en mémoire d'une liste chaînée contenant les nombres `0`, `1` et `2` :
```
.maillon----------------.    .maillon----------------.    .maillon----------------.
|5.          |          |    |12.         |          |    |27.         |          |
| suivant=12 | valeur=0 | -> | suivant=27 | valeur=1 | -> | suivant= 0 | valeur=2 |
|            |          |    |            |          |    |   (NULL)   |          |
'-----------------------'    '-----------------------'    '-----------------------'
      ^
.liste-------.
|            |
|   tete=5   |
|            |
'------------'
```

> **(i)** Une implémentation très complète et commentée de liste chaînée se trouve à `impl/list.h` et `impl/list.c`.

## 6.2 Piles et file (structures de données, pas zones mémoires, je sais c'est chiant)
En termes de structures de données, une *pile* et une *file* sont quasiment identiques et se charactérisent par quatres fonctions :
- `crée` : création d'une nouvelle pile ou d'une nouvelle file
- `detruit` : libération de la mémoire associée à la pile ou à la file
- `empile`/`enfile` : ajout un élément en dernière position
- `dépile`/`défile` : extraction d'un élément en :
    - première position si c'est une pile (principe *FIFO*, *First In First Out* (l'élément le plus ancien est retiré en premier))
    - dernière position si c'est une file (principe *LIFO*, *Last In First Out* (l'élément le plus récent est retiré en premier))

On peut donc implémenter ces structures aussi bien avec des listes chaînées qu'avec des *tableaux dynamiques* (dans le sens *redimensionnables*).

## 6.3 Tas binaire

Un *tas binaire* est une structure de données qui, en conservant ses éléments dans un certain ordre, vise à minimiser le temps nécessaire pour trouver une valeur précise. On associe chacun des éléments à un *noeud* dans un *arbre* et on impose que :
- Chaque noeud ne peut posséder au plus que deux liaisons vers le bas (on appel un tel arbre un *arbre binaire presque complet*).
- Pour chaque noeud `n`, tous les noeuds à gauche de `n` ont une valeur inférieure à celle de `n` et tous les noeuds à droite de `n` ont une valeur supérieure à celle de `n`
```
Représentation d'un arbre avec les valeurs 
a=15, b=8, c=1, d=9, e=28 et f=21 :

                    .a----.
                    |  15 |
                   .'-----'.
                 ,'         ',
               ,'             ',
       .b----.'                 '.e----.
       |  8  |                   |  28 |
       '-----'                   '-----'
       /     \                   /
      /       \                 /
.c----.       .d----.     .f----.
|  1  |       |  9  |     |  21 |
'-----'       '-----'     '-----'

Un parcour en largeur (lecture des noeuds 
de gauche à droite) passerait par :

 c  ->  b  ->  d  ->  a  ->  f  ->  e
(1)    (8)    (9)    (15)   (21)   (28)
```
Ainsi, on peut très rapidement trouver une valeur dans le tas binaire en le traversant depuis la *racine* (noeud tout en haut) et en se demandant à chaque étape : "Ma valeur est-elle plus grande ou plus petite que celle du noeud actuel ?". Si elle est plus grande alors on passe au noeud en bas à droite (aussi appelé *enfant droit*). Sinon, si elle est plus petite, alors on passe au noeud en bas à gauche (aussi appelé *enfant gauche*). Sinon, on est déjà tombé sur le bon noeud et on s'arrête. On aura alors parcouru un maximum de 1 noeud par étage de l'arbre, soit `log2(n)` noeuds avec `n` le nombre de total de noeuds dans l'arbre : la *complexité en temps* pour trouver une valeur dans un tas binaire est en *O(log(n))*

Pour représenter cet arbre en code, on va utiliser un tableau `tas_bin[n]` tel que :
- `tab_bin[0]` soit la racine de l'arbre (noeud tout en haut).
- `tab_bin[1]` soit l'enfant gauche de la racine (en bas à gauche).
- `tab_bin[2]` soit l'enfant droit de la racine (en bas à droite).

Puis, pour un noeud à `tab_bin[i]` :
- `tab_bin[2 * i + 0]` soit son enfant gauche (en bas à gauche).
- `tab_bin[2 * i + 1]` soit son enfant droit (en bas à droite).

En appliquant cet indexage, l'arbre dans l'exemple plus haut devient le tableau suivant :
```
                    .-----------------------.
                    |                       |
            .-------|---------------.       |
            |-------|-------.       |       |
            |       |       V       V       V
,-----------------------------------------------.
|0.     |1.     |2.     |3.     |4.     |5.     |
|  a=15 |  b=8  |  e=28 |  c=1  |  d=9  |  f=21 |
|       |       |       |       |       |       |
'-----------------------------------------------'
    |       ^       ^
    |-------'       |
    '---------------'
```

# 7. Compilation et modularité
## 7.1 Compilation avec GCC, création d'un MAKEFILE
Le compilateur utilisé en cours est le *GNU C Compiler (gcc)*. Il permet de convertir le code source en code machine en spécifiant :
- `-o {output_file}` : L'emplacement du fichier de sortie (qui pourra être exécuté).
- `-I {include_path}` : Les dossiers où se stiuent les libraires utilisées (pas nécessaire pour la librairie standard, donc tous les `#include <...>` vus précédemment).
- `-l {librarie}` : Nom des librairies à utiliser. La plus importante est `-lm` lorsqu'on utilise `#include <math.h>` (dans laquelle se trouvent racine carré et les fonctions trigonométriques).
- `{source_files}` : Liste des fichiers source `.c` à compiler.

Imaginons que nous avons trois fichiers de source `main.c`, `list.c` et `array.c` qui utilisent `<math.h>`. On peut lancer une compilation :
  - Directement depuis le terminal :
    ```sh
    $ gcc -o main main.c list.c array.c -lm
    # ~~~ ~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~ ~~~
    # ^   ^       ^                     ^
    # |   |       |                     Librairie <math.h>
    # |   |       Liste des fichiers sources (.c) à compiler
    # |   Fichier de sortie (exécutable en mettant `./main` dans le terminal)
    # Compilateur C
    ```

  - À l'aide d'une *makefile* en créant un fichier de compilation et en lanceant ce fichier (aussi appelé *script*).  
    Fichier `makefile` (ceci est bien le nom du fichier, c'est nécessaire de le nommer ainsi) :
    ```makefile
    main: main.c array.c list.c
    #~~~  ~~~~~~~~~~~~~~~~~~~~~
    # ^   ^
    # |   Liste de dépendances (voir (i) ci-dessous)
    # Nom de la commande à executer
        gcc -o main main.c list.c array.c -lm
    #   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    #   ^
    #   Commande telle qu'on l'aurait écrite dans le terminal.
    #   /!\ L'INDENTATION EST OBLIGATOIRE /!\ 
    ```
    Lancement de la compilation depuis le terminal :
    ```sh
    $ make
    # ou bien
    $ make main
    ```

    > **(i)** Les dépendances sont les fichiers qui, si ils ont été modifiés depuis la dernière compilation à l'aide de cette commande, nécessitent d'être recompilés. Ceci est pratique si la commande pour compiler est divisée en plusieurs sous-commandes afin de ne pas recompiler tout le projet à chaque fois. Dans le cadre des projets à l'ISMIN, la base de code ne devient jamais assez grande pour que ce soit nécessaire, mais c'est une bonne habitude à prendre pour plus tard.


## 7.2 Modularité
En C, la modularité correspond à la division des fichiers sources pour permettre de compiler en plusieurs blocs et de réutiliser du code. Tout comme depuis le début on utilise `#include <...>` pour importer des librairies standards, on peut créer des librairies manuellement et les importer avec `#include "nom_de_ma_librarie.h"`.

L'idée est donc de séparer dans un fichier source `.c` des *implémentations* communes (par exemple, si on travaille sur une liste chaînée, on peut mettre dans ce `.c` toutes les fonctions qui permettent de créer, libérer et modifier la liste chaînée) et créer un fichier `.h` (aussi appelé *header file* en anglais) dans lequel on va *déclarer* les fonctions et les types qui sont utilisés par cette librarie (toujours sur l'exemple des listes chaînées, ce seraient les structures du maillon et de la liste ainsi que les déclarations des fonctions).

Fichier `header.h` :
```C
// Les trois commandes préprocesseur (commençant par `#`) permettent au compilateur de savoir que 
// ce fichier ne doit être inclu qu'une seule fois (donc si deux fichiers `.c` ont une commande 
// `#include "header.h"`, alors seule une de ces deux commandes va définir le type `MyStruct` et 
// les fonctions `myStructCreate` et `myStructDestroy`, sans quoi le compilateur renverrait une 
// erreur `multiple definitions of "myStructCreate"`)
#ifndef _MACRO_UNIQUE_H_
//    ^ ^
//    | Nom unique associé à ce fichier `.h`. C'est grâce à ce nom unique que le compilateur sait
//    | si ce fichier a déjà été inclu ou non.
//    Vérifie si la macro `_MACRO_UNIQUE_H_` existe déjà. Si elle n'existe
//    pas, tout ce qui se trouve entre `#ifndef` (if not defined) et `#endif` (end if) sera défini.
//    Dans le cas contraire, aucune des déclarations plus bas seront définies.
#define _MACRO_UNIQUE_H_
//      ^
//      Nom unique associé à ce fichier `.h`. Il doit être le même que pour le `#ifndef`

// Un commentaire pour indiquer à quoi sert cette structure
typedef struct MyStruct
{
    int a, b;
    float c;
} MyStruct;

// Un commentaire pour dire ce que fait ma fonction, qu'est-ce qu'elle attend comme arguments et
// qu'est-ce qu'elle renvoie
MyStruct myStructCreate(int valeur_d_initialisation);

// Idem
void myStructDestroy(MyStruct valeur_a_detruire);

#endif
```

Fichier `implementation.c` :
```C
// Ce include va nous donner accès au type `MyStruct` pour qu'il puisse être utilisé dans ce fichier
#include "header.h"

MyStruct myStructCreate(int valeur_d_initialisation)
{
    // Des commentaires pour expliquer le fonctionnement et le raisonnement derrière mon implémentation
    ...
    // C'est ici que tout le code à exécuter (implémentation) doit apparaître et non pas dans le .h !
}

void myStructDestroy(MyStruct valeur_a_detruire)
{
    ...
}
```

Des exemples de modularité sont disponibles dans `impl/` et dans `annales/`.

> **(i)** La macro (ici `_MACRO_UNIQUE_H_`), le nom du fichier source (ici `implementation.c`) et celui du header (ici `header.h`) ne sont pas corélés, dans le sens ou écrire `list.c` et `list.h` ne veut pas automatiquement dire que `list.h` contient les déclarations (structures, types, fonctions) de `list.c`.