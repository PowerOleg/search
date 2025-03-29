
//#include "crawler.h"
//#include "postgres_manager.h"
#include "webloader.h"
#include "file_manager.h"


struct Config
{
	std::string sqlhost;//хост, на котором запущена база данных;
	std::string sqlport;//порт, на котором запущена база данных;
	std::string dbname;//название базы данных;
	std::string username;//имя пользователя для подключения к базе данных;
	std::string password;//пароль пользователя для подключения к базе данных;
	std::string url;//стартовая страница для программы «Паук»;
	std::string crowler_depth;//глубина рекурсии для программы «Паук»;
	std::string http_port;//порт для запуска программы - поисковика.
};

void PrintConsole(std::string text)
{
	std::cout << text << std::endl;
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "ru");
	Config config;
	File_manager file_manager("config.ini");
	file_manager.FillConfig(
		&config.sqlhost,
		&config.sqlport,
		&config.dbname,
		&config.username,
		&config.password,
		&config.url,
		&config.crowler_depth,
		&config.http_port);


	//std::string host = "httpbin.org";
	//std::string target = "/get";
	
	//std::string host = "github.com";
	//std::string target = "/PowerOleg?tab=repositories";
	//https://mail.ru/
	std::string host = "mail.ru";
	std::string target = "/";
	 
	//crowler.Mapping(config.url);
	//Crawler crawler(host, target);
	//crawler.HttpWebSocketRequest();


	//Postgres_manager postgres("localhost", "5432", "dvdrental", "postgres", "106");
	//postgres.Test();
	return 0;
}

// протестировано в MSVS2017
//int main2()
//{
//    system("chcp 1251");
//
//    boost::asio::io_context ioc;
//    Webloader ldr{ ioc };
//
//    std::vector<std::string> vUri
//    {
//        "https://3dnews.ru/" // начальная ссылка
//    };
//
//    std::unordered_set<std::string> ustUsed{ vUri.begin(), vUri.end() }; // для отработанных ссылок
//
//    std::vector<std::vector<std::string>> vres; // найденные ссылки
//
//    int dpth = 2; // глубина обхода
//    while (dpth--)
//    {
//        vres.clear();
//        vres.reserve(vUri.size()); // чтобы не переаллоцировался
//
//        // вектор vUri обрабатывается пулом из 4 потоков
//        boost::asio::thread_pool tpool{ 4 };
//        for (auto const& sUri : vUri)
//        {
//            vres.emplace_back();
//            boost::asio::post(tpool, std::bind(&Loader::load, &ldr, std::cref(sUri), std::ref(vres.back())));
//        }
//        tpool.join();
//
//        // вывод результата
//        removeDuplicatesFragments(vres, ustUsed);
//        std::cout << "\n*********************************** Список из " << vUri.size() << " просмотренных страниц: ****************************************\n\n";
//        for (int i = 0; i < vres.size(); ++i)
//        {
//            std::cout << "Ссылок: " << std::setw(5) << std::left << vres.at(i).size() << " для страницы: " << vUri.at(i) << "\n\n";
//            for (auto const& str : vres.at(i))
//            {
//                //std::cout << str << std::endl; // вывод найденных ссылок
//            }
//        }
//
//        // перед заходом на следующую итерацию перекидываем все найденные ссылки из vres в vUri 
//        absLinks(vUri, vres);
//        vUri.clear();
//        for (auto const& vLnk : vres)
//        {
//            vUri.insert(vUri.end(), vLnk.begin(), vLnk.end());
//        }
//    }
//
//}