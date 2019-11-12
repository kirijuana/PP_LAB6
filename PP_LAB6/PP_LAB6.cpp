#include "stdafx.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <vector>
using namespace std;

int front_1 = 0; //индекс для чтения из буфера
int rear_1 = 0; //индекс для записи в буфер
int front_2 = 0; //индекс для чтения из буфера
int rear_2 = 0; //индекс для записи в буфер
pthread_mutex_t mutex;

pthread_cond_t damage_1;
pthread_cond_t damage_2;
			  //int
pthread_mutex_t mutexD; //мьютекс для операции записи
pthread_mutex_t mutexF; //мьютекс для операции чтения
int k = 100; // расстояние м/у городами
map<int, vector<int>> cannon_1;
map<int, vector<int>> cannon_2;

int z_1 = 0; // кол-во пушек
int z_2 = 0;

// 0 - здоровье пушки // 1 - дальность стрельбы // 2 - точность стрельбы // 3 - урон по области // 4 расположение от начала города

void cannons_generation()
{
	srand(time(NULL));
	z_1 = rand() % 10 + 1; // количество пушек
	for (int i = 0; i < z_1; i++)
	{
		cannon_1[i].push_back(10); // здоровье каждой пушки 10
		for (int j = 1; j < 5; j++)
		{
			if (j == 2)
				cannon_1[i].push_back(rand() % 4); // точность от 0 до 3
			else
				cannon_1[i].push_back(rand() % 10 + 1);
		}
	}


	z_2 = rand() % 10 + 1; // количество пушек
	for (int i = 0; i < z_2; i++)
	{
		cannon_2[i].push_back(10); // здоровье каждой пушки 10
		for (int j = 1; j < 5; j++)
		{
			if (j == 2)
				cannon_2[i].push_back(rand() % 4); // точность от 0 до 3
			else
				cannon_2[i].push_back(rand() % 10 + 1);
		}
	}
}

bool check_damage_2 = true;
bool check_damage = true;
void *Cannons_2(void *param)
{
	pthread_mutex_lock(&mutex);
	if (!check_damage_2)
		front_2 = (front_2 + 1);
	if (front_2 >= z_2 /*|| front_2 + 1 == z_2*/)
	{
		printf("Целых пушек у второго города не осталось, либо оставшиеся пушки не достают до вражеских\n");
		system("pause");
	}
	check_damage_2 = false;
	while (!check_damage_2)
	{
		if (cannon_2[front_2][0] > 0) // если пушка жива, то стреляет
		{
			for (int j = 0; j < z_1; j++)
			{
				if ((cannon_2[front_2][1] + k >= cannon_1[j][4] + k) && (cannon_1[j][0] > 0))
				{
					cannon_1[j][0] = cannon_1[j][0] - cannon_2[front_2][3] * cannon_2[front_2][2];
					check_damage_2 = true;
					if (cannon_1[j][0] <= 0)
					{
						printf("Город #1: пушка № %d взорвана.\n", j + 1);
						while (!check_damage)
						{
							pthread_cond_wait(&damage_2, &mutex);
						}
						break;
					}
				}
			}
			if (!check_damage_2)
				break;
		}
		else
		{
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&damage_1);
	return NULL;
}

void *Cannons_1(void *param)
{
	// int data, i;
	pthread_mutex_lock(&mutex);
	if (!check_damage)
		front_1 = (front_1 + 1);
	if (front_1 >= z_1 /*|| front_1 + 1 == z_1*/)
	{
		printf("Целых пушек у первого города не осталось, либо оставшиеся пушки не достают до вражеских\n");
		system("pause");
	}
	check_damage = false;
	while (!check_damage)
	{
		if (cannon_1[front_1][0] > 0) // если пушка жива, то стреляет
		{
			for (int j = 0; j < z_2; j++)
			{
				if ((cannon_1[front_1][1] + k >= cannon_2[j][4] + k) && (cannon_2[j][0] > 0))
				{
					cannon_2[j][0] = cannon_2[j][0] - cannon_1[front_1][3] * cannon_1[front_1][2];
					check_damage = true;
					if (cannon_2[j][0] <= 0)
					{
						printf("Город #2: пушка № %d взорвана.\n", j + 1);
						while (!check_damage_2)
						{
							pthread_cond_wait(&damage_1, &mutex);
						}
						break;
					}
				}
			}
			
			if (!check_damage)
				break;
		}
		else
		{
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&damage_2);
	return NULL;
}



int main()
{
	int i;
	//инициализация мьютексов и семафоров
	pthread_mutex_init(&mutex, NULL);

	pthread_cond_init(&damage_1, NULL);
	pthread_cond_init(&damage_2, NULL);

	cannons_generation();

	printf("Пушки первого города (%d):\n", z_1);
	for (int i = 0; i < z_1; i++)
	{
		printf("Пушка # %d: %d %d %d %d %d \n", i + 1, cannon_1[i][0], cannon_1[i][1], cannon_1[i][2], cannon_1[i][3], cannon_1[i][4]);
	}

	printf("Пушки второго города (%d):\n", z_2);
	for (int i = 0; i < z_2; i++)
	{
		printf("Пушка # %d: %d %d %d %d %d \n", i + 1, cannon_2[i][0], cannon_2[i][1], cannon_2[i][2], cannon_2[i][3], cannon_2[i][4]);
	}

	//запуск пушек первого города
	pthread_t *threadP = new pthread_t[z_1*2];
	for (i = 0; i<z_1 * 2; i++)
		pthread_create(&threadP[i], NULL, Cannons_1, NULL);
	//запуск пушек второго города
	pthread_t *threadC = new pthread_t[z_2*2];
	for (i = 0; i<z_2*2; i++)
		pthread_create(&threadC[i], NULL, Cannons_2, NULL);

	Cannons_2(NULL);
	system("pause");
}

