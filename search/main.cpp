#include "crowler.h"
#include "postgres_manager.h"
/* LAYOUT
	1) init
	{
		1 подключаемся к sql
		2 boost httpclient
		3 boost server
		4 xml?
	}
	2) запрашиваем одно слово для поиска (в дальнейшем поиск 4 слов)
	3) вносим стартовую страницу в queue
	4) запускаем thread_pool тасков
	 таска делает
	 {
		1 индексируем сайт - это заносим данные в sql
		2 ищем ссылки и вносим в queue
	 }

	thread_pool обрабатывает внесенные таски while queue != 0
	5) вывод результата. статическая страница с первыми 10 самыми релевантными ссылоками на веб-страницы
	*/

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "ru");
	Postgres_manager postgres("localhost", "5432", "dvdrental", "postgres", "106");
	//postgres.Test();

	Crowler crowler;
	crowler.SimpleRequest();

	return 0;
}