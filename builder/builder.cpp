// builder.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

#include <boost/program_options.hpp>

#include "async++.h"

#include <boost/process.hpp>
#include <boost/filesystem.hpp>

using namespace boost::process;

namespace bp = boost::process;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	std::system("chcp 1251");

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "Выводим вспомогательное сообщение")		
		("config,c", po::value<std::string>()->default_value("Debug"), "Указываем конфигурацию сборки (по умолчанию Debug)")
		("install,i", "Добавляем этап установки (в директорию _install)")
		("pack,p", "Добавляем этап упаковки (в архив формата tar.gz)")
		("timeout,t", po::value<int>()->default_value(500), "Указываем время ожидания (в секундах)")
		;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	try
	{
		po::notify(vm);
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	std::string config("-DCMAKE_BUILD_TYPE=Debug");
	int timeout = 0;
	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		return EXIT_SUCCESS;
	}

	//boost::filesystem::path cmake_path = bp::search_path("cmake");
	//std::cout << "cmake_path = " << cmake_path << std::endl;

	if (vm.count("config"))
	{
		config = "-DCMAKE_BUILD_TYPE=" + vm["config"].as<std::string>();
		std::cout << "config" << config.c_str() << std::endl;
	}

	if (vm.count("timeout"))
	{
		timeout = vm["timeout"].as<int>();
		std::cout << "timeout" << config.c_str() << std::endl;
	}	

	auto task1 = async::spawn([]() -> int
	{
		//std::cout << "Task 1 executes asynchronously" << std::endl;
		boost::filesystem::path cmake_path = bp::search_path("cmake");
		std::cout << "cmake_path = " << cmake_path << std::endl;
		bp::system(cmake_path, "-H.", "-B_builds", "-DCMAKE_INSTALL_PREFIX=_install", config, bp::std_out > stdout, bp::std_err > stderr);
	});

	auto task2 = task1.then([](int x) -> int
	{
		//std::cout << "Task 1 executes asynchronously" << std::endl;
		boost::filesystem::path cmake_path = bp::search_path("cmake");
		std::cout << "cmake_path = " << cmake_path << std::endl;
		bp::system(cmake_path, "-H.", "-B_builds", "-DCMAKE_INSTALL_PREFIX=_install", config, bp::std_out > stdout, bp::std_err > stderr);
	});


	return EXIT_SUCCESS;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
