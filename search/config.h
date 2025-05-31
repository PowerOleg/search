#pragma once

#include <string>
namespace crawler
{
	struct Config
	{
		std::string sqlhost;//����, �� ������� �������� ���� ������;
		std::string sqlport;//����, �� ������� �������� ���� ������;
		std::string dbname;//�������� ���� ������;
		std::string username;//��� ������������ ��� ����������� � ���� ������;
		std::string password;//������ ������������ ��� ����������� � ���� ������;
		std::string url;//��������� �������� ��� ��������� �����;
		std::string crawler_depth;//������� �������� ��� ��������� �����;
		std::string http_port;//���� ��� ������� ��������� - ����������.
	};
}