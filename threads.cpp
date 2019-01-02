//Author: Travelle Barrett
//Note: Use command: g++ main.cpp  -std=c++11 -lpthread -o a2 to run on cs machine

//include some stuff

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <iostream>
#include <semaphore.h>
#include <thread>
#include <mutex>

//create variables

int INF = INT_MAX; // maximum number for infinity value
int **graph = NULL; // matrix graph
int nodeCount = 0; //number of nodes in the matrix
int **dist = NULL; //distance between each node

//Synchronization Variables
int readCount = 0; //the read count
std::mutex readCountLock; //the read count lock
sem_t semphoreRW; //create a semaphore

struct args_s{ //create a structure
    int i;
    int k;
    };

void  getGraph(){
    //Get nodes from the user
   while(true){
        std::cout << "Enter Your Node:";
        std::cin >> nodeCount;
        if (nodeCount < 1 ){
            std::cout << "Node Count Must Be greater than 0";
            std::cout << std::endl;
        }
        else {
            break;
        }
    }
   int verticesCount = 0;
   while(true){ //Get number of vertices from the user
        std::cout << "Enter Your Vertice Count:";
        std::cin >> verticesCount;
        if (verticesCount < 1 ){
            std::cout << "vertices Count Must Be greater than 0";
            std::cout << std::endl;
        }
        else {
            break;
        }
    }
   //Allocate Graph
   graph = (int**)malloc(sizeof (int*) * nodeCount);
   for (int i = 0 ; i < nodeCount ; i++){
       graph[i] = (int*) malloc(sizeof (int) *nodeCount);
   }
   //Now Get Vertice Count
   for (int i = 0 ; i < nodeCount ; i ++){
       for (int j = 0 ; j < nodeCount ;j ++){
           if (i == j ){
               graph[i][j] = 0;
           }
           else {
               graph[i][j] = INF;
           }
       }
   }
   int* vertice = (int*)malloc(sizeof (int) * 3 );
   for (int i = 0 ; i < verticesCount ; i++){
       std::cout << "Enter Vertices  (" << i+1 << ") : ";
       std::cin >> vertice[0];
       std::cin >> vertice[1];
       std::cin >> vertice[2];
       std::cout << std::endl;
       if (vertice[2] < 0){
           std::cout << "You Can Do that Weight negative";
           i--;
       }
       graph[vertice[0]-1][vertice[1]-1] = vertice[2];
       graph[vertice[1]-1][vertice[0]-1] = vertice[2];
   }
}

void floydWarshallWorker(args_s input){ //Floyd-Warshall worker method
    int i = input.i;
    int k = input.k;
    for (int j = 0 ; j < nodeCount;  j++) {
       readCountLock.lock(); //Grab Read Lock So No Other Read Thread Can Modify/Read From The Read Count
       readCount++; //Increment The Read Count
       if (readCount == 1){ //Check To See If First Thread In Matrix To Read
           sem_wait(&semphoreRW); //If it is then try to stop writing threads from writing
       }
       readCountLock.unlock(); //Let other Reading Threads In To Read/Write To ReadCount
       if (dist[i][k] != INF && dist[k][j] != INF &&
               dist[i][k] + dist[k][j] < dist[i][j]){
           {
               //Done Reading
               readCountLock.lock();
               readCount--;
               if (readCount == 0 ){
                   sem_post(&semphoreRW);
               }
               readCountLock.unlock();
           }//Switch From Read To Writing
           //Time To Write
           sem_wait(&semphoreRW); //now wait
           dist[i][j] = dist[i][k] + dist[k][j];
           sem_post(&semphoreRW);
       }
       else {
           //Done Reading
           readCountLock.lock(); //lock the read count lock
           readCount--; //decrement read count
           if (readCount == 0 ){
               sem_post(&semphoreRW); //now post
           }
           readCountLock.unlock(); //unlock read count lock
       }
    }
}

void floydWarshall(){ //create function for Floyd-Warshall
    std::thread* threadFW = (std::thread*)malloc(sizeof (std::thread) * nodeCount);
    for (int k = 0 ; k < nodeCount;  k++) {
        for (int i = 0 ; i < nodeCount;  i++){
            args_s input;
            input.i = i; //pass parameters from structure
            input.k = k;
            threadFW[i] =std::move(std::thread(floydWarshallWorker,input));
        }
        for (int thread= 0; thread < nodeCount ; thread++){
            threadFW[thread].join();//join the threads
        }
    }
}

int main() //create driver method
{
    getGraph(); //call the get graph fucntion
    sem_init(&semphoreRW,false, 1); //initialize semaphore
    //Allocate the distance
    dist = (int**)malloc(sizeof (int*) * nodeCount);
    for (int i = 0 ; i < nodeCount; i++){
        dist[i] = (int*) malloc(sizeof (int) *nodeCount);
    }
    for (int i = 0 ; i < nodeCount ; i ++){
        for (int j = 0 ; j < nodeCount; j++){
          dist[i][j] = graph[i][j];
        }
    }
    //Call FloydWarshall() method here
    floydWarshall();
    //Print the Graph
    std::cout << "\nInitial Distance Matrix:\n\n";
    for (int i = 0 ; i < nodeCount; i++){
        for (int j = 0; j < nodeCount; j++){
			if (graph[i][j] == INF) { 
				std::cout << "INF "; //print INF
			}
			else {
				std::cout << graph[i][j] << " "; //print the graph
			}
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    //Print the Distance
    std::cout << "Final Distance Matrix:\n\n";
    for (int i = 0 ; i < nodeCount ; i ++){
        for (int j = 0 ; j < nodeCount; j++){
            if (dist[i][j]  == INF)   { //check if distance is equal to infinity
                 std::cout <<  "INF "; //print INF
            }
            else {
                 std::cout <<  dist[i][j] << " "; //print the distance
            }
        }
        std::cout << std::endl;
    }
}
