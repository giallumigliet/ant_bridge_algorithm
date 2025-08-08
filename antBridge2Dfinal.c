#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define NUM_ANT 15
#define NUM_DRONE_ANT 2
#define ENV_X 9
#define ENV_Y 9
#define ANT_FORCE 1 //the ant can have on his back max 1 other ants in the state MOVING, otherwise the bridge will break
#define PRINTF_SPEED 50000 //microseconds (100000 microseconds = 100 milliseconds = 0.1seconds)
#define PERC_NO_OBSTACLES 7 //7 over 10, so the probability is 70% ground and 30% obstacles

struct Ant{
    int ID; //to identify the ant
    int positionX; //wrt the starting point
    int positionY;
    int state; //0=MOVING, 1=STILL, 2=WARNING
};

struct Position{
    int x;
    int y;
};


void printPheromoneMap(struct Ant *ant, int environment[ENV_X][ENV_Y], int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y], bool returned, int bestPath){
    int value = 0;
    printf("\nPheromone Map:\n");
    for (int iY = 0; iY < ENV_Y; iY++) {
        for (int iX = 0; iX < ENV_X; iX++) {
            if(returned){
                if(pheromoneMap[bestPath][iX][iY] == 0){
                    if (environment[iX][iY] == 3) printf("\033[38;5;208m T  \033[0m");
                    else if (environment[iX][iY] == 4) printf("\033[34m N  \033[0m");
                    else printf("%2d  ", pheromoneMap[bestPath][iX][iY]);
                }    
                else printf("\033[34m%2d  \033[0m", pheromoneMap[bestPath][iX][iY]);
            }
            else{
                for(int i = 0; i < NUM_DRONE_ANT; i++){
                    value += pheromoneMap[i][iX][iY];
                }
                if(value == 0){
                    if (environment[iX][iY] == 3) printf("\033[38;5;208m T  \033[0m");
                    else if (environment[iX][iY] == 4) printf("\033[34m N  \033[0m");
                    else printf("%2d  ", value);
                }    
                else printf("\033[34m%2d  \033[0m", value);
            }
            value = 0;  
        }
        printf("\n");
    }
}



void printTerrainWithAnts(struct Ant *ant, int environment[ENV_X][ENV_Y], int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y], bool returned, int bestPath){
    //clear the console
    system("clear");
    // number of ants per position
    int occupations[ENV_X][ENV_Y];

    for(int indY=0; indY < ENV_Y; indY++){  
        for(int indX=0; indX < ENV_X; indX++){  
            occupations[indX][indY] = 0;
        }
    }

    int i = 0;
    while(i < NUM_ANT){ 
        occupations[ant[i].positionX][ant[i].positionY]++;
        i++;
    }
    //prints the array with environment and ants
    printf("\nEnvironment with ants:\n");

    for (int iY = 0; iY < ENV_Y; iY++) {
        for (int iX = 0; iX < ENV_X; iX++) {
            if (occupations[iX][iY] > 0) printf("\033[34m%2d  \033[0m", occupations[iX][iY]);
            else{
                if (environment[iX][iY] == 1) printf("\033[32m X  \033[0m");    //ground
                else if (environment[iX][iY] == 2) printf("\033[31m =  \033[0m");   //obstacle
                else if (environment[iX][iY] == 3) printf("\033[38;5;208m T  \033[0m");    //target
                else if (environment[iX][iY] == 4) printf("\033[34m N  \033[0m");    //nest
                else printf(" .  ");
            } 
        }
        printf("\n");
    }
    //just the environment
    printf("\nEnvironment:\n");
    for (int iY = 0; iY < ENV_Y; iY++) {
        for (int iX = 0; iX < ENV_X; iX++) {
            if (environment[iX][iY] == 1) printf("\033[32m X  \033[0m");    //ground
            else if (environment[iX][iY] == 2) printf("\033[31m =  \033[0m");   //obstacle
            else if (environment[iX][iY] == 3) printf("\033[38;5;208m T  \033[0m");    //target
            else if (environment[iX][iY] == 4) printf("\033[34m N  \033[0m");    //nest
            else printf(" .  ");
        }
        printf("\n");
    }

    printPheromoneMap(ant, environment, pheromoneMap, returned, bestPath);
    
    usleep(PRINTF_SPEED);
}



bool OverWeightDetection(struct Ant *ant, int currentAnt){
    int counterMoving = 0;
    int counterStill = 0;
    bool overWeight = false;
    int i = 0;
    while(i < NUM_ANT){ //OVERWEIGHT DETECT
        if(ant[i].state == 0 && ant[i].positionX == ant[currentAnt].positionX && ant[i].positionY == ant[currentAnt].positionY) counterMoving++;
        else if((ant[i].state == 1 || ant[i].state == 2) && ant[i].positionX == ant[currentAnt].positionX && ant[i].positionY == ant[currentAnt].positionY) counterStill++;
        i++;
    }
    if(counterMoving > (counterStill*ANT_FORCE)) overWeight = true;
    return overWeight;
}


bool NoWeightDetection(struct Ant *ant, int currentAnt){
    int counter = 0;
    bool noWeight = false;
    int i = 0;
    while(i < NUM_ANT){ //NO WEIGHT DETECT
        if(ant[i].state == 0 && ant[i].positionX == ant[currentAnt].positionX && ant[i].positionY == ant[currentAnt].positionY) counter++;
        i++;
    }
    if(counter == 0) noWeight = true;
    return noWeight;
}

bool WarningDetection(struct Ant *ant, int currentPositionX, int currentPositionY){
    bool warning = false;
    int i=0;
    while(i<NUM_ANT){ //WARNING DETECT
        if(ant[i].state == 2 && ant[i].positionX == currentPositionX && ant[i].positionY == currentPositionY) warning = true;
        i++;
    }
    return warning;
}

int WarningAntDetection(struct Ant *ant, int currentPositionX, int currentPositionY){
    int antWarning;
    int i=0;
    while(i<NUM_ANT){ //WARNING DETECT
        if(ant[i].state == 2 && ant[i].positionX == currentPositionX && ant[i].positionY == currentPositionY) antWarning = i;
        i++;
    }
    return antWarning;
}

void StopWarning(struct Ant *ant, int currentAnt){
    if(!OverWeightDetection(ant, currentAnt)){
        ant[WarningAntDetection(ant, ant[currentAnt].positionX, ant[currentAnt].positionY)].state = 1;
        //printf("UPDATE: ant %d in position %d isn't anymore in warning state\n", currentAnt, ant[currentAnt].position);
    }
}

bool AntBridgePresent(struct Ant *ant, int currentAnt, int checkPositionX, int checkPositionY){
    bool present = false;
    int i=0;
    while(i<NUM_ANT){ 
        if((ant[i].state == 1 || ant[i].state == 2) && ant[i].positionX == checkPositionX && ant[i].positionY == checkPositionY && i != currentAnt) present = true;
        i++;
    }
    return present;
}

bool HoleDetection(struct Ant *ant, int currentAnt, struct Position nextStep, int environment[ENV_X][ENV_Y]){
    bool hole = false;
    if(environment[nextStep.x][nextStep.y] == 0 && !AntBridgePresent(ant, currentAnt, nextStep.x, nextStep.y)) hole = true;     //if the next position is hole
    return hole;
}

bool BridgeDetection(struct Ant *ant, int currentAnt){      //to verify if the ant is in the middle of a bridge and is necessary, in that case it can't move
    bool bridge = false;
    int i=0;
    while(i<NUM_ANT){ 
        if(AntBridgePresent(ant, currentAnt, (ant[currentAnt].positionX), (ant[currentAnt].positionY - 1)) || AntBridgePresent(ant, currentAnt, (ant[currentAnt].positionX), (ant[currentAnt].positionY + 1)) || AntBridgePresent(ant, currentAnt, (ant[currentAnt].positionX - 1), (ant[currentAnt].positionY)) || AntBridgePresent(ant, currentAnt, (ant[currentAnt].positionX +1), (ant[currentAnt].positionY)))
            if(!AntBridgePresent(ant, currentAnt, (ant[currentAnt].positionX), (ant[currentAnt].positionY))) bridge = true;
        i++;
    }
    
    return bridge;
}


void printStates(struct Ant *ant){
    int i = NUM_DRONE_ANT-1;
    printf("FINAL STATES:\n");

    while(i < NUM_ANT){ 
        if(ant[i].state == 0) printf("ant %d, position: (%d,%d) in state: MOVING\n", i, ant[i].positionX, ant[i].positionY);
        else if(ant[i].state == 1) printf("ant %d, position: (%d,%d) in state: STILL\n", i, ant[i].positionX, ant[i].positionY);
        else if(ant[i].state == 2) printf("ant %d, position: (%d,%d) in state: WARNING\n", i, ant[i].positionX, ant[i].positionY);
        i++;
    }
}


struct Position ChooseNest(struct Ant *ant, int environment[ENV_X][ENV_Y], int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y], bool returned, int bestPath){
    int x = 0;
    int y = 0;
    bool setted = false;
    struct Position nest;
    

    printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
    printf("INFO: nest and target can be set only on the ground X!!\n");

    while(!setted){
        printf("Select coordinates for the NEST: ");
        scanf("%d", &x);
        scanf("%d", &y);

        nest.x = x;
        nest.y = y;

        if(environment[x][y] == 1){     //setting the nest
            environment[x][y] = 4;
            for(int i=0; i<NUM_ANT; i++){       //initializing the ants in the nest
                ant[i].positionX = x;
                ant[i].positionY = y;
            }
            setted = true;
            printf("OK nest in (%d, %d)\n", x, y);
            printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
        }
    }
    return nest;
}

int max(int a, int b, int c){
    int max_val = a;
    if(b > max_val) max_val = b;
    if(c > max_val) max_val = c;
    return max_val;
}

//void TargetSmellIntensity(int targetSmellMap[ENV_X][ENV_Y], int x, int y){
//    int distance = 0;
//    int posX = 0;
//    int posY = 0;
//    while(posY < ENV_Y) {
//        while(posX < ENV_X) {
//            distance = abs(posX - x) + abs(posY - y);        //distance calculation
//            targetSmellMap[posX][posY] = ENV_X + ENV_Y - distance;      //setting the intensity values (higher near the target)
//            posX++;
//            distance = 0;
//        }
//        posX = 0;
//        posY++;
//    }
//}

void TargetSmellIntensityObstacles(int targetSmellMap[ENV_X][ENV_Y], int x, int y, int environment[ENV_X][ENV_Y]){
    int min = 100 * (ENV_X + ENV_Y);    //set min very high
    bool finish = false;
    bool minfound = false;
    int iX = 0;
    int iY = 0;

    while(iY < ENV_Y) {
        while(iX < ENV_X) {       
            targetSmellMap[iX][iY] = min;      //setting all the intensity values high
            iX++;
        }
        iX = 0;
        iY++;
    }
    
    targetSmellMap[x][y] = (ENV_X + ENV_Y)*(20-PERC_NO_OBSTACLES)/10;     //set intensity of target

    //iterations to expand the smell through the obstacles
    while(!finish && min > 0){
        iX = 0;
        iY = 0;
        finish = true;
        while(iY < ENV_Y) {
            while(iX < ENV_X) {
                if(targetSmellMap[iX][iY] == (100 * (ENV_X + ENV_Y)) && environment[iX][iY] != 2) finish = false;   //check if all the map is updated
                if(targetSmellMap[iX][iY] < min && !minfound){      //search the minimum value
                    min = targetSmellMap[iX][iY];
                    minfound = true;
                }
                if(targetSmellMap[iX][iY] == min && minfound){      //if the minimum has been found update all the cells around the minima (just if there isn't any obstacle and it isn't updated yet)
                    if(targetSmellMap[iX][iY-1] == (100 * (ENV_X + ENV_Y)) && environment[iX][iY-1] != 2 && iY > 0) targetSmellMap[iX][iY-1] = min - 1;   //nord
                    if(targetSmellMap[iX][iY+1] == (100 * (ENV_X + ENV_Y)) && environment[iX][iY+1] != 2 && iY < ENV_Y-1) targetSmellMap[iX][iY+1] = min - 1;   //sud
                    if(targetSmellMap[iX+1][iY] == (100 * (ENV_X + ENV_Y)) && environment[iX+1][iY] != 2 && iX < ENV_X-1) targetSmellMap[iX+1][iY] = min - 1;   //est
                    if(targetSmellMap[iX-1][iY] == (100 * (ENV_X + ENV_Y)) && environment[iX-1][iY] != 2 && iX > 0) targetSmellMap[iX-1][iY] = min - 1;   //ovest
                }

                iX++;
            }
            iX = 0;
            iY++;
        }
        minfound = false;
    }
    
    //setting all the remaining cells to intensity zero
    iX = 0;
    iY = 0;
    while(iY < ENV_Y) {
        while(iX < ENV_X) {       
            if(targetSmellMap[iX][iY] == (100 * (ENV_X + ENV_Y))) targetSmellMap[iX][iY] = 0;                 
            iX++;
        }
        iX = 0;
        iY++;
    }
}


void LayPheromone(int selectedMap, int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y], int x, int y){
    int distance = 0;
    int posX = 0;
    int posY = 0;
    pheromoneMap[selectedMap][x][y] += 1;        //laying pheromone

    //while(posY < ENV_Y) {
    //    while(posX < ENV_X) {
    //        distance = abs(posX - x) + abs(posY - y);        //distance from laying spot
    //        if(pheromoneMap[x][y]-distance >= 0) pheromoneMap[posX][posY] += (pheromoneMap[x][y] - distance);      //pheromone smell spreads around
    //        posX++;
    //        distance = 0;
    //    }
    //    posX = 0;
    //    posY++;
    //}
}



struct Position ChooseTarget(struct Ant *ant, int environment[ENV_X][ENV_Y], int targetSmellMap[ENV_X][ENV_Y], int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y], bool returned, int bestPath){
    int x1 = 0;
    int y1 = 0;
    struct Position target;
    bool setted = false;

    while(!setted){
        printf("Select coordinates for the TARGET: \n");
        scanf("%d", &x1);
        scanf("%d", &y1);

        if(environment[x1][y1] == 1){     //setting the target
            environment[x1][y1] = 3;
            printf("OK target in (%d, %d)\n", x1, y1);
            printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
            //TargetSmellIntensity(targetSmellMap, x1, y1);
            TargetSmellIntensityObstacles(targetSmellMap, x1, y1, environment);

            printf("\n\nSmell Intensity Map:\n");
            for (int iY = 0; iY < ENV_Y; iY++) {
                for (int iX = 0; iX < ENV_X; iX++) {
                    if(environment[iX][iY] == 3) printf("\033[38;5;208m%2d  \033[0m", targetSmellMap[iX][iY]);
                    else if(environment[iX][iY] == 2) printf("\033[31m%2d  \033[0m", targetSmellMap[iX][iY]);
                    else printf("%2d  ", targetSmellMap[iX][iY]);
                }
                printf("\n");
            }

            target.x = x1;
            target.y = y1;
            setted = true;
            return target;
        }
    }
    
}



bool RunSimulation(){
    int run = 0;
    int start = false;
    printf("\nPress 1 to run the simulation...");         
    scanf("%d", &run);
    if(run == 1) start = true;          //running simulation
    else{
        printf("wrong number, simulation not run!!\n");        //ERROR
        RunSimulation();
    } 
};


struct Position SmellDirection(int x, int y, int targetSmellMap[ENV_X][ENV_Y], int environment[ENV_X][ENV_Y]){      //smelling the ant chooses the direction with higher smell intensity
    struct Position nextPos;
    int maxVal = targetSmellMap[x][y];
    int change1 = 0; 
    int change2 = 0;
    int change3 = 0;
    
    change1 = rand() % 2;
    change2 = rand() % 2;
    change3 = rand() % 2;

    if (x > 0 && targetSmellMap[x-1][y] > maxVal && environment[x-1][y] != 2) {  // Ovest
        maxVal = targetSmellMap[x-1][y];
        nextPos.x = x-1;
        nextPos.y = y;
    }
    if (x < ENV_X-1 && targetSmellMap[x+1][y] >= maxVal && environment[x+1][y] != 2) {  // Est
        if (targetSmellMap[x+1][y] > maxVal) {  // se il valore è maggiore si aggiorna
            maxVal = targetSmellMap[x+1][y];
            nextPos.x = x+1;
            nextPos.y = y;
        }else if(targetSmellMap[x+1][y] == maxVal && change1){   //se il valore massimo è uguale può scegliere random se aggiornare
            maxVal = targetSmellMap[x+1][y];
            nextPos.x = x+1;
            nextPos.y = y;
        }
    }
    if (y > 0 && targetSmellMap[x][y-1] >= maxVal && environment[x][y-1] != 2) {  // Nord
        if (targetSmellMap[x][y-1] > maxVal) {  
            maxVal = targetSmellMap[x][y-1];
            nextPos.x = x;
            nextPos.y = y-1;
        }else if(targetSmellMap[x][y-1] == maxVal && change2){
            maxVal = targetSmellMap[x][y-1];
            nextPos.x = x;
            nextPos.y = y-1;
        }

    }
    if (y < ENV_Y-1 && targetSmellMap[x][y+1] >= maxVal && environment[x][y+1] != 2) {  // Sud
        if (targetSmellMap[x][y+1] > maxVal) {  
            maxVal = targetSmellMap[x][y+1];
            nextPos.x = x;
            nextPos.y = y+1;
        }else if(targetSmellMap[x][y+1] == maxVal && change3){   
            maxVal = targetSmellMap[x][y+1];
            nextPos.x = x;
            nextPos.y = y+1;
        }
        
    }
    return nextPos;
}



struct Position SmellPheromone(int selectedMap, int x, int y, int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y], struct Position *prevPos){      //smelling the ant chooses next step on the pheromone trial, trying not to go back
    struct Position nextPos;
    
    if (x > 0 && ((x-1) != prevPos->x) && pheromoneMap[selectedMap][x-1][y] > 0) {  // Ovest
        nextPos.x = x-1;
        nextPos.y = y;
    }
    if (x < ENV_X-1 && ((x+1) != prevPos->x) && pheromoneMap[selectedMap][x+1][y] > 0) {  // Est
        nextPos.x = x+1;
        nextPos.y = y;
    }
    if (y > 0 && ((y-1) != prevPos->y) && pheromoneMap[selectedMap][x][y-1] > 0) {  // Nord
        nextPos.x = x;
        nextPos.y = y-1;
    }
    if (y < (ENV_Y-1) && ((y+1) != prevPos->y) && pheromoneMap[selectedMap][x][y+1] > 0) {  // Sud
        nextPos.x = x;
        nextPos.y = y+1;
    }

    prevPos->x = x;
    prevPos->y = y;
    //printf("\nx:%d, y:%d\n",x,y);
    //printf("\nprev:%d,%d, next:%d,%d\n",prevPos->x,prevPos->y,nextPos.x,nextPos.y);
    return nextPos;
}





int main(){
    
    //creating and initializing ants
    struct Ant ant[NUM_ANT];

    for(int i=0; i<NUM_ANT; i++){    //setting all ant as worker
        ant[i].ID = 0;             //ID: 0=worker ant, 1=drone ant
        ant[i].positionX = 0;
        ant[i].positionY = 0;
        ant[i].state = 0;
    }

    int holesDetected[NUM_DRONE_ANT];   //number of holes detected in the path registered by each drone ant
    
    for(int i=0; i<NUM_DRONE_ANT; i++){    //setting first NUM_DRONE_ANT as drone ant
        ant[i].ID = 1;
        holesDetected[i] = 0;  //initializing to zero
    }

    
    

    //creating environment 1=ground, 0=hole
    int environment[ENV_X][ENV_Y];
    int targetSmellMap[ENV_X][ENV_Y];
    int pheromoneMap[NUM_DRONE_ANT][ENV_X][ENV_Y];
    int numHoles = 0;
    srand(time(NULL));

    for(int iy=0; iy < ENV_Y; iy++){  
        for(int ix=0; ix < ENV_X; ix++){  
            environment[ix][iy] = rand() % 2;  //0=hole, 1=ground, 2=obstacle
            
            if(environment[ix][iy] == 1 && (rand() % 10)>(PERC_NO_OBSTACLES-1)) environment[ix][iy] = 2; //if it is ground it has almost a 30% possibility of being an obstacle
            if(environment[ix][iy] == 0) numHoles++;

            for(int i2=0; i2<NUM_DRONE_ANT; i2++){    
                pheromoneMap[i2][ix][iy] = 0;       //initialization to zero of the pheromone maps for all drone ants
            }
        }
    }

    
    //program
    bool movingPresence = true;
    bool found = false;
    bool returned = false;
    int bestPath = 0;
    int currentAnt = 0;
    int antFound = 0;
    int antReturned = 0;
    int antGoal = 0;
    struct Position nextStep;
    struct Position prevPos[NUM_ANT];
    struct Position nest = ChooseNest(ant, environment, pheromoneMap, returned, bestPath);
    struct Position target = ChooseTarget(ant, environment, targetSmellMap, pheromoneMap, returned, bestPath);

    for(int i=0; i<NUM_ANT; i++){    //setting all ant previous positions for SmellPheromone function
        prevPos[i].x = nest.x;
        prevPos[i].y = nest.y;
    }

    if(!RunSimulation())
    

    printf("\033[1;31m\nFASE 1: DRONE ANTS SEARCHING FOR FOOD\033[0m\n");

    while(!found){
        
        currentAnt = rand() % NUM_DRONE_ANT;  //choosing random drone ant
        if(ant[currentAnt].positionX != target.x || ant[currentAnt].positionY != target.y){
            nextStep = SmellDirection(ant[currentAnt].positionX, ant[currentAnt].positionY, targetSmellMap, environment);
            LayPheromone(currentAnt, pheromoneMap, ant[currentAnt].positionX, ant[currentAnt].positionY);       //all drone ants lay pheromone to be able to return to the nest
            ant[currentAnt].positionX = nextStep.x;
            ant[currentAnt].positionY = nextStep.y;
            if(environment[ant[currentAnt].positionX][ant[currentAnt].positionY] == 0) holesDetected[currentAnt]++; //number of holes detection

            if(ant[currentAnt].positionX == target.x && ant[currentAnt].positionY == target.y) antFound++;
            //printf("ant %d moves to position %d \n", currentAnt, ant[currentAnt].position);
            printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
            printf("\nNew coord: %d, %d", nextStep.x, nextStep.y);
            usleep(500000);
        }
        
        if(antFound == NUM_DRONE_ANT) found = true;
    }
    
    printf("\033[1;31m\nFASE 2: DRONE ANTS RETURNING HOME BY FOLLOWING THEIR OWN PHEROMONE PATH\033[0m\n");
    sleep(4);
    for(int i=0; i<NUM_ANT; i++){    //setting all ant previous positions for SmellPheromone function
        prevPos[i].x = ant[i].positionX;
        prevPos[i].y = ant[i].positionY;
    }
    while(!returned){
        
        currentAnt = rand() % NUM_DRONE_ANT;  //choosing random drone ant
        
        if(ant[currentAnt].positionX != nest.x || ant[currentAnt].positionY != nest.y){     //if current drone not returned
            LayPheromone(currentAnt, pheromoneMap, ant[currentAnt].positionX, ant[currentAnt].positionY);    //the drone ants that found the food will lay pheromone
            nextStep = SmellPheromone(currentAnt, ant[currentAnt].positionX, ant[currentAnt].positionY, pheromoneMap, &prevPos[currentAnt]);
            ant[currentAnt].positionX = nextStep.x;
            ant[currentAnt].positionY = nextStep.y;
            
            if(ant[currentAnt].positionX == nest.x && ant[currentAnt].positionY == nest.y) antReturned++;
            //printf("ant %d moves to position %d \n", currentAnt, ant[currentAnt].position);
            printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
            printf("\nNew coord: %d, %d", nextStep.x, nextStep.y);
        }
        usleep(500000);
        if(antReturned == NUM_DRONE_ANT) returned = true;
    }





    
    //stats control, choosing the best (less number of holes)
    int bestPheromoneMap[ENV_X][ENV_Y];
    int minHoles = ENV_X * ENV_Y;
    

    for(int i=0; i<NUM_DRONE_ANT; i++){    
        if(holesDetected[i] < minHoles){
            minHoles = holesDetected[i];
            bestPath = i;
        } 
        printf("\nthe ant %d, detected %d holes\n", i, holesDetected[i]);
        printPheromoneMap(ant, environment, pheromoneMap, returned, i);
    }
    printf("\nTHE BEST PATH IS OF THE ANT %d, WITH %d HOLES", bestPath, minHoles);
    sleep(8);
    




    
    printf("\033[1;31m\nFASE 3: WORKER ANTS FOLLOWING PHEROMONE PATH FOR THE TARGET\033[0m\n");
    sleep(4);
    for(int i=0; i<NUM_ANT; i++){    //setting all ant previous positions for SmellPheromone function
        prevPos[i].x = ant[i].positionX;
        prevPos[i].y = ant[i].positionY;
    }
    while(movingPresence){
        
        currentAnt = NUM_DRONE_ANT + rand() % (NUM_ANT-NUM_DRONE_ANT);  //choosing random worker ant
        //printf("\nSTATE: %d\n", ant[currentAnt].state);
        if(ant[currentAnt].state == 2){     //just to show the warning
            printf("\033[31m WAAAAAAAAAAAAARNING!\033[0m");
            usleep(500000);
        }
        
        if(ant[currentAnt].state == 0 && (ant[currentAnt].positionX != target.x || ant[currentAnt].positionY != target.y)){     //if the ant is MOVING
            
            if(WarningDetection(ant, ant[currentAnt].positionX, ant[currentAnt].positionY)){    //state update to STILL due to warning
                ant[currentAnt].state = 1;
                //printf("UPDATE: ant %d in position %d answers to warning and stays still helping for the bridge\n", currentAnt, ant[currentAnt].position);
                StopWarning(ant, currentAnt);           
            }
            else{
                nextStep = SmellPheromone(bestPath, ant[currentAnt].positionX, ant[currentAnt].positionY, pheromoneMap, &prevPos[currentAnt]);    //choosing direction to take
                
                if(HoleDetection(ant, currentAnt, nextStep, environment)){    //state update to STILL due to hole
                    ant[currentAnt].positionX = nextStep.x;
                    ant[currentAnt].positionY = nextStep.y;
                    ant[currentAnt].state = 1;
                    //printf("UPDATE: ant %d moves to position %d and stays still forming a bridge to cover the hole\n", currentAnt, ant[currentAnt].position);
                    printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
                    printf("\nNEW COORD: %d, %d", nextStep.x, nextStep.y);
                }
                else if(!HoleDetection(ant, currentAnt, nextStep, environment)){  //if there is ground and no warnings the ant moves forward
                    ant[currentAnt].positionX = nextStep.x;
                    ant[currentAnt].positionY = nextStep.y;
                    if(ant[currentAnt].positionX == target.x && ant[currentAnt].positionY == target.y) antGoal++;
                    //printf("ant %d moves to position %d \n", currentAnt, ant[currentAnt].position);
                    printTerrainWithAnts(ant, environment, pheromoneMap, returned, bestPath);
                    printf("\nNew coord: %d, %d", nextStep.x, nextStep.y);
                }

            }

        }
        else if(ant[currentAnt].state == 1){    //if the ant is STILL

            if(OverWeightDetection(ant, currentAnt)){   //state update to WARNING
                ant[currentAnt].state = 2; 
                //printf("UPDATE: ant %d in position %d is in warning state\n", currentAnt, ant[currentAnt].position);
            } 
            
            if(NoWeightDetection(ant, currentAnt) && !BridgeDetection(ant, currentAnt)){     //state update to MOVING
               ant[currentAnt].state = 0;  
               //printf("UPDATE: ant %d in position %d now can move\n", currentAnt, ant[currentAnt].position);
            }

        }
        else if(ant[currentAnt].state == 2){    //if the ant is in WARNING

            if(!OverWeightDetection(ant, currentAnt)){   //state update to STILL
                ant[currentAnt].state = 1; 
                //printf("UPDATE: ant %d in position %d isn't anymore in warning state\n", currentAnt, ant[currentAnt].position);
            } 
        }


        movingPresence = false;
        for(int j=NUM_DRONE_ANT; j<NUM_ANT; j++){
            if(ant[j].state == 0 && (ant[j].positionX != target.x || ant[j].positionY != target.y)){
                movingPresence = true;
            } 
        }

    }
    sleep(2);
    printf("\033[1;31m\n\nMOST OF THE ANTS HAVE REACHED THE TARGET!\033[0m\n");
    printStates(ant);
}