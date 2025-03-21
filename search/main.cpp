#include <iostream>
#include "crowler.h"

int main(int argc, char** argv)
{
	Crowler crowler;
	/*1init
	{
	1подключаемся к sql
	2подключаемся к сайту
	}
	2вносим стартовую страницу в queue 
	3запускаем thread_pool тасков
	 таска делает 
	 {
		1индексируем сайт - это заносим данные в sql
		2ищем ссылки и вносим в queue
	 }
	
	thread_pool обрабатывает внесенные таски while queue != 0*/

	return 0;
}