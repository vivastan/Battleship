#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* command line arguments:
    1 - test "T"
*/

#define VERSION "2.01"
#define DESCR "BATTLESHIP Player vs Computer game"
/* new in this version:
    - possibility to edit ship positions
    - possibility to automatically set player's ships
*/

#define MAXSIZE 16
#define MAXSHIPS 6
#define HORIZONTAL 0
#define VERTICAL 1
#define COMPUTER 2
#define PLAYER 3

/* ascii codes of chars for hit/miss/sink */
#define HIT 177
#define MISS 120 /* 155 */
#define SINK 219

/* colours */
#define WHITE 0
#define YELLOW 33
#define BLUE 34

typedef struct _pair
{
    int x;
    int y;
} pair;

/* declarations of functions */
pair newPair(int x, int y);
void settings(int test, int *size, int *shipNum);
void setField(int test, int who, int autoSet, int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum);
void printField(int test, int guessedSize, pair *guessed, int field[][2 * MAXSIZE], int fieldSize);
void printShipsLen(int test, int shipNum, int sunkShips[]);
int orientation(pair ships[][2], int ship);
int checkIfSunk(int test, int who, pair coord, int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum, int *ship);
void updateSunkShip(int test, int ship, int who, int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum, int *guessedSize, pair **guessed, int *colorSize, pair **color);
void testPrint(int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum);
void add2SunkShips(int test, int ship, pair ships[][2], int *sunkShips);
void computerGuess(int test, pair *coord, int *guessedSize, pair **guessed, int fieldSize, int field[][2 * MAXSIZE], int turn);
int checkHorizontal(int test, pair *coord, int fieldSize, int field[][2 * MAXSIZE]);
int checkVertical(int test, pair *coord, int fieldSize, int field[][2 * MAXSIZE]);
int readExit(char *input);

int main(int argc, char **argv)
{
    int test = 0;
    /* processing input parameters */
    if (argc > 2)
    {
        printf("\nerr: incorrect number of input parameters");
        exit(1);
    }
    if (argc == 2 && !strcmp(argv[1], "T"))
        test = 1;
    printf("\nVERSION %s: %s\n", VERSION, DESCR);

    printf("\nDescription of game");
    printf("\nBattleship is a strategy type guessing game for two players. It is played on ruled grids on which each player's fleet of warships are marked. The locations of the fleets are concealed from the other player. Players alternate turns calling \"shots\" at the other player's ships, and the objective of the game is to destroy the opposing player's fleet.");
    printf("\nIf the computer guesses the position of your ship, the colour changes.");
    printf("\nYou can exit the game by typing \"exit\" in any input field.");
    printf("\nGood luck!\n");

    /* setup */
    int maxsize = 0, maxships = 0;
    printf("\nsettings:");
    settings(test, &maxsize, &maxships);
    printf("\nchosen field size: %dx%d", maxsize, maxsize);
    printf("\nchosen number of ships in each fleet: %d\n", maxships);

    int i, j, sunkComp = 0, sunkPlayer = 0, turn = 1, guessSize = 0, autoSet = -1;
    int field[MAXSIZE][2 * MAXSIZE] = {{0}};
    /* field of ships - initialization (memorising start and end coordinates of each ship) */
    pair shipsPos[2 * MAXSHIPS][2] = {{{0}}};
    int sunkCompShips[MAXSHIPS] = {0};
    int sunkPlayerShips[MAXSHIPS] = {0};
    pair *guessed = NULL;
    time_t t;
    srand((unsigned)time(&t));

    /* set ships for computer's field */
    setField(test, COMPUTER, 0, field, maxsize, shipsPos, maxships);
    char input[10];
    while (autoSet == -1)
    {
        printf("\nlet the computer automatically set coordinates for ships in your fleet Y/N : ");
        if (readExit(input))
            exit(0);
        if (!strcasecmp(input, "Y"))
            autoSet = 1;
        else if (!strcasecmp(input, "N"))
            autoSet = 0;
        else
        {
            printf("\nerr: incorrect input format");
            autoSet = -1;
        }
    }
    /* player sets field */
    setField(test, PLAYER, autoSet, field, maxsize, shipsPos, maxships);

    if (test)
    {
        printf("\n");
        testPrint(field, maxsize, shipsPos, maxships);
    }

    int k, player = 1;
    pair *coordInput = (pair *)malloc(2 * sizeof(pair));
    int inputSize = 2;
    coordInput[1] = newPair(-1, -1);

    printf("\nThe fleets are set!");
    printf("\nMeaning of symbols in the field:\n");
    printf("\tempty - position hasn't been guessed yet\n");
    printf("\t%c - no ship here\n", MISS);
    printf("\t%c - part of the ship is here\n", HIT);
    printf("\t%c - whole ship is sunk\n\n", SINK);
    while (sunkComp < maxships && sunkPlayer < maxships)
    {
        printf("\nturn %d\nsizes of ships left for the player to sink: ", turn);
        printShipsLen(test, maxships, sunkCompShips);
        printf("\nsizes of ships left for the computer to sink: ");
        printShipsLen(test, maxships, sunkPlayerShips);

        printf("\ninput coordinates for your guess (two numbers divided with space - eg. 2 2): ");
        char input[10];
        if (readExit(input))
            exit(0);
        if (sscanf(input, "%d %d", &coordInput[0].y, &coordInput[0].x) != 2)
        {
            printf("\nerr: incorrect input format");
            continue;
        }

        /* translating coordinates into array indexes */
        (coordInput[0].x)--;
        (coordInput[0].y)--;

        /* handle error */
        if (coordInput[0].x >= maxsize || coordInput[0].y >= maxsize || coordInput[0].x <= -1 || coordInput[0].y <= -1)
        {
            printf("\nerr: incorrect coordinates");
            continue;
        }
        else if (field[coordInput[0].x][coordInput[0].y] >= 2)
        {
            printf("\nerr: field has already been guessed");
            continue;
        }

        /* update field */
        if (field[coordInput[0].x][coordInput[0].y])
            field[coordInput[0].x][coordInput[0].y] = 3;
        else
            field[coordInput[0].x][coordInput[0].y] = 2;

        computerGuess(test, &coordInput[1], &guessSize, &guessed, maxsize, field, turn);
        if (test)
        {
            int i;
            printf("\nalready guessed:");
            for (i = 0; i < guessSize; i++)
                printf("\n(%d, %d)", guessed[i].x, guessed[i].y - maxsize);
            printf("\n\ncomputer guessed: (%d, %d)", coordInput[1].x, coordInput[1].y - maxsize);
        }

        if (field[coordInput[1].x][coordInput[1].y]) /* 1 or 5 */
            field[coordInput[1].x][coordInput[1].y] = 3;
        else
            field[coordInput[1].x][coordInput[1].y] = 2;

        if (test)
            testPrint(field, maxsize, shipsPos, maxships);

        int ship = -1;
        /* look in computer's field if player guessed */
        if (checkIfSunk(test, COMPUTER, coordInput[0], field, maxsize, shipsPos, maxships, &ship))
        {
            int zero = 0;
            updateSunkShip(test, ship, COMPUTER, field, maxsize, shipsPos, maxships, &zero, NULL, &inputSize, &coordInput);
            add2SunkShips(test, ship, shipsPos, sunkCompShips);
            sunkComp++;
        }
        ship = -1;
        /* look in player's field if computer guessed */
        if (checkIfSunk(test, PLAYER, coordInput[1], field, maxsize, shipsPos, maxships, &ship))
        {
            if (test)
            {
                printf("\nship %d", ship);
                testPrint(field, maxsize, shipsPos, maxships);
            }
            updateSunkShip(test, ship, PLAYER, field, maxsize, shipsPos, maxships, &guessSize, &guessed, &inputSize, &coordInput);
            if (test)
                printf("\nsize = %d", guessSize);
            add2SunkShips(test, ship, shipsPos, sunkPlayerShips);
            sunkPlayer++;
        }

        printField(test, inputSize, coordInput, field, maxsize);
        turn++;
        if (inputSize > 2)
        {
            coordInput = (pair *)realloc(coordInput, 2 * sizeof(pair));
            inputSize = 2;
        }
    } /* end while */

    if (sunkPlayer == sunkComp)
        printf("\nit's a draw!");
    else
    {
        char who[10] = "player";
        if (sunkPlayer == maxships)
            strcpy(who, "computer");
        printf("\nthe %s won!", who);
    }
    printf("\nnumber of turns: %d\n\n", turn - 1);

    free(guessed);
    free(coordInput);

    return 0;
}

int readExit(char *input)
{
    char in[10];
    if (scanf(" %[^\n]", in) != 1)
        exit(4);
    if (!strcmp(in, "exit"))
    {
        printf("\nexiting program");
        return 1;
    }
    strcpy(input, in);
    return 0;
}

void settings(int test, int *size, int *shipNum)
{
    if (test)
        printf("\nfunction: settings");
    int optSize, optShipNum;
    char input[10];
    *size = 0;
    while (!(*size))
    {
        printf("\nchoose field size:");
        printf("\n\t1 - field 6x6");
        printf("\n\t2 - field 8x8");
        printf("\n\t3 - field 10x10");
        printf("\noption: ");
        if (readExit(input))
            exit(0);
        if (sscanf(input, "%d", &optSize) != 1)
        {
            printf("\nerr: incorrect input format");
            continue;
        }
        switch (optSize)
        {
        case 1:
            *size = 6;
            *shipNum = 3;
            break;
        case 2:
            *size = 8;
            break;
        case 3:
            *size = 10;
            break;
        default:
            printf("\nerr: incorrect option!");
        }
    }

    /* relation between field size and number of ships:
           size        possible number of ships
            6                   3
            8                  4, 5
            10                 5, 6
    */
    int shipSizeDep[4][2] = {
        {3, 0}, /* size = 6 */
        {4, 5}, /* size = 8 */
        {5, 6}, /* size = 10 */
    };
    int i;
    while (!(*shipNum))
    {
        printf("\nchoose number of ships:");
        for (i = 0; i < 2 && shipSizeDep[optSize - 1][i]; i++)
        {
            printf("\n\t%d - %d", i + 1, shipSizeDep[optSize - 1][i]);
            printf(" ships");
        }
        printf("\noption: ");
        if (readExit(input))
            exit(0);
        if (sscanf(input, "%d", &optShipNum) != 1)
        {
            printf("\nerr: incorrect input format");
            continue;
        }
        if (optShipNum > 3 || optShipNum <= 0 || !shipSizeDep[optSize - 1][optShipNum - 1])
            printf("\nerr: incorrect option!");
        else
            *shipNum = shipSizeDep[optSize - 1][optShipNum - 1];
    }
}

void setField(int test, int who, int autoSet, int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum)
{
    if (test)
        printf("\nfunction: setField");

    /* initializing random number generator */
    time_t t;
    srand((unsigned)time(&t));
    int i, j, x, y, start = shipNum + 1, end = 2;
    if (who == PLAYER && !autoSet)
    {
        printf("\nSet starting positions for all ships in your fleet.");
        printf("\nYou will be able to edit set coordinates before starting the game :)");
    }

field_setter:
    for (i = start; i >= end; i--)
    {
        int posx = -1, posy = -1, rot = -1;
        if (test)
            testPrint(field, fieldSize, ships, shipNum);
        int look = 1;

        /* computer "chooses" random coordinates and rotation */
        if (who == COMPUTER || autoSet)
        {
            rot = rand() % 2;
            if (rot == HORIZONTAL)
            {
                posx = rand() % (fieldSize);
                posy = rand() % (fieldSize - i + 1);
            }
            else
            {
                posx = rand() % (fieldSize - i + 1);
                posy = rand() % (fieldSize);
            }
        }
        /* player sets coordinates */
        else
        {
            while (posx == -1 || posy == -1)
            {
                printf("\nset coordinates for ship of length = %d", i);
                printf("\ninput start coordinates for ship (two numbers divided with space - eg. 2 2): ");
                char input[10];
                if (readExit(input))
                    exit(0);
                if (sscanf(input, "%d %d", &posy, &posx) != 2)
                {
                    printf("\nerr: incorrect input format");
                    posx = posy = -1;
                }
                /* handle error */
                if (posx <= 0 || posy <= 0 || posx > fieldSize || posy > fieldSize)
                {
                    printf("\nerr: incorrect coordinates");
                    posx = posy = -1;
                }
            }
            while (rot == -1)
            {
                printf("\ninput rotation (H - horizontal / V - vertical): ");
                char input[10];
                if (readExit(input))
                    exit(0);
                if (!strcasecmp(input, "H"))
                    rot = HORIZONTAL;
                else if (!strcasecmp(input, "V"))
                    rot = VERTICAL;
                else
                {
                    printf("\nerr: incorrect input format");
                    rot = -1;
                }
            }
            /* check if ship lies out of the playing field */
            if (rot == HORIZONTAL && posy >= fieldSize - i + 2)
            {
                printf("\nerr: incorrect coordinates - ship lies out of the playing field\nanother input needed");
                i++;
                continue;
            }
            else if (rot == VERTICAL && posx >= fieldSize - i + 2)
            {
                printf("\nerr: incorrect coordinates - ship lies out of the playing field\nanother input needed");
                i++;
                continue;
            }
            /* setting coordinates to match array iterator */
            posx--;
            posy += fieldSize - 1;
        }

        if (autoSet)
            posy += fieldSize;

        /* check if ships are crossing - only by orientation */
        if (i != shipNum + 1)
        {
            if (test)
                printf("\ncheck if ships are crossing");
            if (rot == HORIZONTAL)
            {
                if (test)
                    printf("\nrot = horizontal");
                for (y = posy; y < posy + i; y++)
                {
                    if (field[posx][y])
                    {
                        if (who == PLAYER && !autoSet)
                            printf("\nerr: ship is crossing another ship set earlier\nanother input needed");
                        look = 0;
                        break;
                    }
                }
                if (test)
                    printf("\n");
            }
            else
            {
                if (test)
                    printf("\nrot = vertical");
                for (x = posx; x < posx + i; x++)
                {
                    if (field[x][posy])
                    {
                        if (who == PLAYER && !autoSet)
                            printf("\nerr: ship is crossing another ship set earlier\nanother input needed");
                        look = 0;
                        break;
                    }
                }
            }
            if (!look)
            {
                i++;
                continue;
            }
        }

        /* adding to array with locations of all ships */
        if (test)
            printf("\n(%d, %d), r = %d, l = %d", posx, posy, rot, i);
        if (rot == HORIZONTAL)
        {
            for (y = posy; y < posy + i; y++)
            {
                if (who == PLAYER)
                    field[posx][y] = 5;
                else
                    field[posx][y] = 1;
            }
            /* insert into array of ships */
            int tmpi = i;
            if (who == PLAYER)
            {
                tmpi = i + shipNum;
                posy -= fieldSize;
                y -= fieldSize;
            }
            ships[tmpi - 2][0] = newPair(posx, posy);
            ships[tmpi - 2][1] = newPair(posx, y - 1);
        }
        else
        {
            for (x = posx; x < posx + i; x++)
            {
                if (who == PLAYER)
                    field[x][posy] = 5;
                else
                    field[x][posy] = 1;
            }
            int tmpi = i;
            if (who == PLAYER)
            {
                tmpi = i + shipNum;
                posy -= fieldSize;
                y -= fieldSize;
            }
            /* insert into array of ships */
            ships[tmpi - 2][0] = newPair(posx, posy);
            ships[tmpi - 2][1] = newPair(x - 1, posy);
        }

        if (who == PLAYER && !autoSet)
        {
            printField(test, 0, NULL, field, fieldSize);
        }
    }
    if (who == PLAYER)
    {
        if (autoSet) printField(test, 0, NULL, field, fieldSize);

        int edit = -1, *tmpSunk = (int *)malloc(shipNum * sizeof(int)), t;
        char input[10];
        for (t = 0; t < shipNum; t++)
            tmpSunk[t] = 0;
        while (edit == -1)
        {
            printf("\nedit locations of set ships Y/N : ");
            if (readExit(input))
                exit(0);
            if (!strcasecmp(input, "Y"))
            {
                edit = 1;
                char input2[10];
                int shipLen = 0;
                while (1)
                {
                    printf("\nship lenghts are: ");
                    printShipsLen(test, shipNum, tmpSunk);
                    printf("\ninput length of ship which you want to edit the location of: ");
                    if (readExit(input2))
                        exit(0);
                    if (sscanf(input2, "%d", &shipLen) != 1)
                        printf("\nerr: incorrect input format");
                    if (shipLen < 2 || shipLen > shipNum + 1)
                        printf("\nerr: ship of length %d doesn't exist\nanother input needed", shipLen);
                    else
                    {
                        start = end = shipLen;
                        break;
                    }
                }
            }
            else if (!strcasecmp(input, "N"))
                edit = 0;
            else
                printf("\nerr: incorrect input format");
        }
        if (edit == 1)
        {
            /* automatic setting of ship coordinates yes / no */
            char input[10];
            autoSet = -1;
            while (autoSet == -1)
            {
                printf("\nautomatically set ship's coordinates Y/N : ");
                if (readExit(input))
                    exit(0);
                if (!strcasecmp(input, "Y"))
                    autoSet = 1;
                else if (!strcasecmp(input, "N"))
                    autoSet = 0;
                else
                {
                    printf("\nerr: incorrect input format");
                    autoSet = -1;
                }
            }
            /* delete ship from array ships and field */
            int k, ship = start + shipNum - 2, a = ships[ship][0].x, b = ships[ship][0].y;
            printf("\nship of length %d currently starting from position (%d, %d) and lies ", start, a + 1, b + 1);
            b += fieldSize;
            if (orientation(ships, ship) == HORIZONTAL)
            {
                printf("horizontally");
                for (; b <= ships[ship][1].y + fieldSize; b++)
                {
                    if (test)
                        printf("\n(a, b) = (%d, %d) with value %d", a, b, field[a][b]);
                    field[a][b] = 0;
                }
            }
            else
            {
                printf("vertically");
                for (; a <= ships[ship][1].x; a++)
                {
                    if (test)
                        printf("\n(a, b) = (%d, %d) with value %d", a, b, field[a][b]);
                    field[a][b] = 0;
                }
            }
            for (k = 0; k < 2; k++)
                ships[ship][k] = newPair(0, 0);
            goto field_setter;
        }
        free(tmpSunk);
    }
    return;
}

/*
meaning of integers in array
0 - no ship here (field hasn't been revealed yet)
1 - part of ship here (field hasn't been revealed yet)
2 - revealed field, no ship here
3 - revealed field, part of ship is here
4 - whole ship is sunk
5 - player is (in the process of) setting coordinates
*/
void printField(int test, int guessedSize, pair *guessed, int field[][2 * MAXSIZE], int fieldSize)
{
    if (test)
        printf("\nfunction: printField");
    int i, j, k, tab;
    if (test && guessedSize > 0)
    {
        printf("\033[1;%dm", BLUE);
        printf("\nguessed pairs: (%d, %d), (%d, %d)", guessed[0].x, guessed[0].y, guessed[1].x, guessed[1].y);
        printf("\033[%dm", WHITE);
    }
    printf("\n\n");
    for (tab = 4; tab < fieldSize / 2; tab++)
        printf("\t");
    printf("\tcomputer's field");
    for (tab = 1; tab < fieldSize / 2; tab++)
        printf("\t");
    printf("player's field\n");
    for (i = 0; i < 2 * fieldSize + 2; i++)
    {
        for (j = 0; j < 4 * fieldSize + 4; j++)
        {
            if (i == 0 || i == 2 * fieldSize + 2)
            {
                if (j == 1)
                    printf("  ");
                else if (j == 2 * fieldSize + 2)
                    printf("\t  ");
                else if (j != 1 && j != 2 * fieldSize + 3 && j % 2)
                    printf("%4d", (j / 2) % (fieldSize + 1));
            }
            else if (j == 2 * fieldSize + 2)
            {
                printf("\t");
                if (i % 2)
                    printf("   ");
                else
                    printf("%2d ", i / 2);
            }
            else if (j == 0)
            {
                if (i % 2)
                    printf("   ");
                else
                    printf("%2d ", i / 2);
            }
            else if (i % 2)
            {
                if (j % 2)
                    printf("+");
                else
                    printf(" - ");
            }
            else if (j % 2)
            {
                printf("|");
            }
            else
            {
                int x = i / 2 - 1, y = j / 2 - 1;
                if (j >= 2 * fieldSize + 2)
                    y--;

                /* colour setup */
                for (k = 0; k < guessedSize; k++)
                    if ((x == guessed[k].x && y == guessed[k].y))
                        printf("\033[1;%dm", BLUE);

                if (!field[x][y] || field[x][y] == 1)
                    printf("   ");
                else if (field[x][y] == 2)
                    printf(" %c ", MISS);
                else if (field[x][y] == 3)
                    printf(" %c ", HIT);
                else if (field[x][y] == 4)
                    printf(" %c ", SINK);
                else
                    printf("\033[1;%dm %c ", YELLOW, HIT);
                printf("\033[%dm", WHITE);
            }
        }
        printf("\n");
    }
}

void printShipsLen(int test, int shipNum, int sunkShips[])
{
    int first = 1, k;
    for (k = 0; k < shipNum; k++)
    {
        if (!sunkShips[k] && first)
        {
            printf("%d", k + 2);
            first = 0;
        }
        else if (!sunkShips[k])
            printf(", %d", k + 2);
    }
}

/* check if whole ship is sunk */
/* ship is a pointer because it turns into index of ships array for the sunk ship, otherwise -1 */
int checkIfSunk(int test, int who, pair coord, int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum, int *ship)
{
    if (test)
        printf("\nfunction: checkIfSunk");
    int i = 0, j, k, t = 0, end = shipNum;
    *ship = -1;
    if (who == PLAYER)
    {
        i = shipNum;
        end = 2 * shipNum;
        coord.y -= fieldSize;
    }
    for (; i < end && !t; i++)
    {
        t = 0;
        for (j = 0; j < 2 && !t; j++)
        {
            if (coord.y == ships[i][j].y || coord.x == ships[i][j].x)
            {
                t = 1;
                break;
            }
        }

        if (t)
        {
            int posy = ships[i][0].y; /* starting position */
            if (orientation(ships, i) == HORIZONTAL)
            { /* horizontally */
                int yend = ships[i][1].y;
                if (who == PLAYER)
                {
                    posy += fieldSize;
                    yend += fieldSize;
                }
                for (k = posy; k <= yend; k++)
                {
                    if (field[ships[i][0].x][k] != 3)
                    {
                        t = 0;
                        break;
                    }
                }
            }
            else
            { /* vertically */
                int xend = ships[i][1].x;
                if (who == PLAYER)
                    posy += fieldSize;
                for (k = ships[i][0].x; k <= xend; k++)
                {
                    if (field[k][posy] != 3)
                    {
                        t = 0;
                        break;
                    }
                }
            }
            if (!t)
                continue;
            *ship = i;
        }
    }

    if (*ship == -1)
        return 0;
    return 1;
}

void updateSunkShip(int test, int ship, int who, int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum, int *guessedSize, pair **guessed, int *colorSize, pair **color)
{
    if (test)
        printf("\nfunction: updateSunkShip");
    int i, k;
    pair *newArr = NULL, *newColor = *color;
    newColor = (pair *)realloc(newColor, (*colorSize + ((ship % shipNum) + 2)) * sizeof(pair));

    int posy = ships[ship][0].y;
    if (orientation(ships, ship) == HORIZONTAL)
    { /* horizontally */
        int yend = ships[ship][1].y;
        if (who == PLAYER)
        { /* translating to indexes of array field */
            posy += fieldSize;
            yend += fieldSize;
        }
        for (k = posy; k <= yend; k++)
        {
            field[ships[ship][0].x][k] = 4;
            newColor[*colorSize] = newPair(ships[ship][0].x, k);
            (*colorSize)++;
            for (i = 0; i < *guessedSize; i++)
                if ((*guessed)[i].x == ships[ship][0].x && (*guessed)[i].y == k)
                    (*guessed)[i].x = -5;
        }
    }
    else
    { /* vertically */
        int xend = ships[ship][1].x;
        if (who == PLAYER)
            posy += fieldSize;
        for (k = ships[ship][0].x; k <= xend; k++)
        {
            field[k][posy] = 4;
            newColor[*colorSize] = newPair(k, posy);
            (*colorSize)++;
            for (i = 0; i < *guessedSize; i++)
                if ((*guessed)[i].x == k && (*guessed)[i].y == posy)
                    (*guessed)[i].x = -5;
        }
    }
    *color = newColor;

    if (who == COMPUTER)
        return;

    newArr = (pair *)realloc(newArr, (*guessedSize - ship + 2 + shipNum) * sizeof(pair));

    for (k = 0, i = 0; i < *guessedSize; i++)
    {
        if ((*guessed)[i].x != -5)
        {
            newArr[k] = (*guessed)[i];
            k++;
        }
    }
    *guessed = newArr;
    *guessedSize -= ship + 2 - shipNum;
    *guessed = (pair *)realloc(newArr, (*guessedSize) * sizeof(pair));
}

void testPrint(int field[][2 * MAXSIZE], int fieldSize, pair ships[][2], int shipNum)
{
    printf("\nfunction: testPrint\n");
    int i, j;
    for (i = 0; i < fieldSize; i++)
    {
        for (j = 0; j < 2 * fieldSize; j++)
        {
            if (j == fieldSize)
                printf("\t");
            printf("%d ", field[i][j]);
        }
        printf("\n");
    }

    printf("\n");
    for (i = 0; i < shipNum; i++)
    {
        for (j = 0; j < 2; j++)
            printf("%d%d", ships[i][j].x, ships[i][j].y);
        printf("\t");
        for (j = 0; j < 2; j++)
            printf("%d%d", ships[i + shipNum][j].x, ships[i + shipNum][j].y);
        printf("\n");
    }
}

void add2SunkShips(int test, int ship, pair ships[][2], int *sunkShips)
{
    if (test)
        printf("\nfunction: add2SunkShips");
    int len;
    if (orientation(ships, ship) == HORIZONTAL)
        len = ships[ship][1].y - ships[ship][0].y;
    else
        len = ships[ship][1].x - ships[ship][0].x;
    sunkShips[len - 1] = 1;
}

pair newPair(int x, int y)
{
    pair tmp = {x, y};
    return tmp;
}

int orientation(pair ships[][2], int ship)
{
    if (ships[ship][0].x == ships[ship][1].x)
        return HORIZONTAL;
    return VERTICAL;
}

/* algorithm for guessing player's ships (no cheating) */
void computerGuess(int test, pair *coord, int *guessedSize, pair **guessed, int fieldSize, int field[][2 * MAXSIZE], int turn)
{
    if (test)
        printf("\nfunction: computerGuess");
    time_t t;
    srand((unsigned)time(&t));
    int where = -1, newx = -1, newy = -1, i;
    pair *newArr = NULL, tmp;

    if (*guessed)
        newArr = *guessed;

    for (i = 0; i < *guessedSize; i++)
    {
        if (!i)
        {
            newx = newArr[i].x;
            newy = newArr[i].y;
        }
        else
        {
            if ((*guessed)[i].x == newx && ((*guessed)[i].y == newy + 1 || (*guessed)[i].y == newy - 1))
                where = HORIZONTAL;
            else if ((*guessed)[i].y == newy && ((*guessed)[i].x == newx - 1 || (*guessed)[i].x == newx + 1))
                where = VERTICAL;
        }
        if (where != -1)
            break;
    }

    if (newx != -1)
    {
        if (where == -1)
            where = rand() % 2;
        tmp = newPair(newx, newy);
    }
    else
        goto guess;

    if (test)
        printf("\nwhere = %d, starting coordinates: (%d, %d)", where, newx, newy - fieldSize);

    if (where == HORIZONTAL && checkHorizontal(test, &tmp, fieldSize, field) == HORIZONTAL)
        goto guess;
    if (checkVertical(test, &tmp, fieldSize, field) == VERTICAL)
        goto guess;
    checkHorizontal(test, &tmp, fieldSize, field);

guess:
    pair *random = (pair *)malloc(3 * sizeof(pair)); /* will store 6 random numbers in case first two cases fail */
    if (test)
        printf("\nguess:\nwhere = %d", where);
    if (where == -1)
    {
        for (i = 0; i < 3; i++)
        {
            newx = rand() % fieldSize;
            newy = rand() % fieldSize;
            /* setting coordinates to be in the player's part of the field */
            newy += fieldSize;
            random[i] = newPair(newx, newy);
        }
        newx = -1;
    }
    else
    {
        newx = tmp.x;
        newy = tmp.y;
    }

    for (i = 0; i < 3; i++)
    {
        int tmpx = random[i].x, tmpy = random[i].y;
        if (test)
            printf("\n(tmpx, tmpy) = (%d, %d)", tmpx, tmpy);
        /* for a more logical way of guessing - if there's a field in the square around, don't try guessing that field */
        if (newx == -1 && turn <= fieldSize / 2 * fieldSize)
        {
            if (tmpx != 0)
                if (field[tmpx - 1][tmpy] != 5 && field[tmpx - 1][tmpy] >= 2)
                    continue;
            if (tmpx < fieldSize - 1)
                if (field[tmpx + 1][tmpy] >= 2 && field[tmpx + 1][tmpy] != 5)
                    continue;
            if (tmpy != 0)
                if (field[tmpx][tmpy - 1] >= 2 && field[tmpx][tmpy - 1] != 5)
                    continue;
            if (tmpy < fieldSize * 2 - 1)
                if (field[tmpx][tmpy + 1] >= 2 && field[tmpx][tmpy + 1] != 5)
                    continue;
        }
        /* if ALL fields around the field currently guessing are already guessed (a ship of length 1 doesn't exist) */
        else if (newx == -1 && turn > fieldSize / 2 * fieldSize)
        {
            int stop = 0;
            if (tmpx != 0)
                if (!(field[tmpx - 1][tmpy] != 5 && field[tmpx - 1][tmpy] >= 2))
                    stop = 1;
            if (!stop && tmpx < fieldSize - 1)
                if (!(field[tmpx + 1][tmpy] >= 2 && field[tmpx + 1][tmpy] != 5))
                    stop = 1;
            if (!stop && tmpy != 0)
                if (!(field[tmpx][tmpy - 1] >= 2 && field[tmpx][tmpy - 1] != 5))
                    stop = 1;
            if (!stop && tmpy < fieldSize * 2 - 1)
                if (!(field[tmpx][tmpy + 1] >= 2 && field[tmpx][tmpy + 1] != 5))
                    stop = 1;

            if (stop)
                continue;
        }

        if (newx == -1)
        {
            newx = random[i].x;
            newy = random[i].y;
        }
        if (field[newx][newy] < 2 || field[newx][newy] == 5)
        {
            *coord = newPair(newx, newy);
            break;
        }
        else if (field[newx][newy] >= 2 && field[newx][newy] != 5 && i == 2)
        {
            newx = -1;
            goto guess;
        }
    }

    if (newx == -1)
        goto guess;

    if (test)
        printf("\nwhere = %d\nguessing: (%d, %d)\nvalue is %d", where, newx, newy - fieldSize, field[newx][newy]);

    /* if there's a part of a ship in that field (value = 1 or 5) */
    if (field[newx][newy])
    {
        newArr = (pair *)realloc(newArr, (*guessedSize + 1) * sizeof(pair));
        newArr[*guessedSize] = newPair(newx, newy);
        *guessed = newArr;
        (*guessedSize)++;
    }

    free(random);

    return;
}

int checkHorizontal(int test, pair *coord, int fieldSize, int field[][2 * MAXSIZE])
{
    if (test)
        printf("\nfunction: checkHorizontal");
    int j, x = coord->x, y = coord->y;
    if (test)
        printf("\ncheckHorizontal in: (%d, %d)", x, y - fieldSize);
    /* check RIGHT if a field has been guessed already */
    if (test)
        printf("\nlook RIGHT");
    for (j = y + 1; j < 2 * fieldSize; j++)
        if (field[x][j] != 3)
            break;
    if (test)
        printf("\ncurr coordinates (%d, %d) with value %d", x, j - fieldSize, field[x][j]);
    if (j < 2 * fieldSize && (field[x][j] < 2 || field[x][j] == 5))
    {
        coord->y = j;
        return HORIZONTAL;
    }

    /* check LEFT if a field has been guessed already */
    if (test)
        printf("\nlook LEFT");
    for (j = y - 1; j >= fieldSize; j--)
        if (field[x][j] != 3)
            break;
    if (test)
        printf("\ncurr coordinates (%d, %d) with value %d", x, j - fieldSize, field[x][j]);
    if (j >= fieldSize && (field[x][j] < 2 || field[x][j] == 5))
    {
        coord->y = j;
        return HORIZONTAL;
    }
    return VERTICAL;
}

int checkVertical(int test, pair *coord, int fieldSize, int field[][2 * MAXSIZE])
{
    if (test)
        printf("\nfunction: checkVertical");
    int i, x = coord->x, y = coord->y;
    if (test)
        printf("\ncheckVertical in: (%d, %d)", x, y - fieldSize);
    /* check UP if a field has been guessed already */
    if (test)
        printf("\nlook UP");
    for (i = x - 1; i >= 0; i--)
        if (field[i][y] != 3)
            break;
    if (test)
        printf("\ncurr coordinates (%d, %d) with value %d", i, y - fieldSize, field[i][y]);
    if (i >= 0 && (field[i][y] < 2 || field[i][y] == 5))
    {
        coord->x = i;
        return VERTICAL;
    }

    /* check DOWN */
    if (test)
        printf("\nlook DOWN");
    for (i = x + 1; i < fieldSize; i++)
        if (field[i][y] != 3)
            break;
    ;
    if (test)
        printf("\ncurr coordinates (%d, %d) with value %d", i, y - fieldSize, field[i][y]);
    if (i < fieldSize && (field[i][y] < 2 || field[i][y] == 5))
    {
        coord->x = i;
        return VERTICAL;
    }
    return HORIZONTAL;
}