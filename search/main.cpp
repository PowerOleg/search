#include "crowler.h"
#include "postgres_manager.h"
/* LAYOUT
	1) init
	{
		1 ������������ � sql
		2 boost httpclient
		3 boost server
		4 xml?
	}
	2) ����������� ���� ����� ��� ������ (� ���������� ����� 4 ����)
	3) ������ ��������� �������� � queue
	4) ��������� thread_pool ������
	 ����� ������
	 {
		1 ����������� ���� - ��� ������� ������ � sql
		2 ���� ������ � ������ � queue
	 }

	thread_pool ������������ ��������� ����� while queue != 0
	5) ����� ����������. ����������� �������� � ������� 10 ������ ������������ ��������� �� ���-��������
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