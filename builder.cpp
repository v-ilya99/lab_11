// builder.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

#include <boost/program_options.hpp>

#include "async++.h"

#include <boost/process.hpp>
#include <boost/filesystem.hpp>

namespace bp = boost::process;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	//std::cout << "Hello World!\n";


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

	std::string cmake_path;
	int result_code = 333;

	bool is_install = false;

	boost::filesystem::path path = bp::search_path("cmake");
	cmake_path = path.string();
	//std::cout << "CMake full filename[prepare]: " << cmake_path << std::endl;
	//
	//std::cout << "Cmake version ..." << std::endl;
	//std::cout << "cmake_path = " << cmake_path << std::endl;
	//std::vector<std::string> args;
	//args.push_back("--version");
	//
	////int result = bp::system(path, "--version");
	//
	//
	//bp::child c(path, bp::args(args));
	//if (c.joinable())
	//	c.join();
	//result_code = c.exit_code();
	//std::cout << "result_code[out] = " << result_code << std::endl; // << result << std::endl;//
	//std::cout << std::endl;
	//
	//std::system("pause");

	//auto builder_task = async::spawn([=]() mutable
	//{
	//	std::cout << "Find CMake ..." << std::endl;
	//	boost::filesystem::path path = bp::search_path("cmake");
	//	cmake_path = path.generic_string();
	//	std::cout << "CMake full filename: " << cmake_path << std::endl;
	//
	//	result_code = cmake_path.empty();
	//}).share();


	//if (vm.count("pack"))
	//{
	//	builder_task.then([cmake_path, result_code](async::shared_task<void>) mutable
	//	{
	//		if (result_code)
	//			return;
	//
	//		std::cout << "Cmake pack ..." << std::endl;
	//		std::vector<std::string> args;
	//		args.push_back("--target ");
	//		args.push_back("package");
	//		bp::child c(cmake_path, bp::args(args), bp::std_out > stdout, bp::std_err > stderr);
	//		result_code = c.exit_code();
	//	});
	//}
	//
	
	//
	

	if (vm.count("config"))
	{
		config = "-DCMAKE_BUILD_TYPE=" + vm["config"].as<std::string>();
		std::cout << "[config] = " << config.c_str() << std::endl;
	}
	//
	////builder_task.then([](async::shared_task<void>)
	auto builder_task = async::spawn([cmake_path, config, &result_code]() mutable
	{
		std::cout << "CMake full filename[prepare]: " << cmake_path << std::endl;
		
		std::cout << "Cmake prepare ..." << std::endl;
		std::cout << "cmake_path = " << cmake_path << std::endl;
		std::cout << "config = " << config << std::endl;
		std::vector<std::string> args;
		args.push_back("-H.");
		args.push_back("-B_builds");
		args.push_back("-DCMAKE_INSTALL_PREFIX=_install");
		args.push_back(config);
		bp::child c(cmake_path, bp::args(args));
		if (c.joinable())
			c.join();
		result_code = c.exit_code();
		std::cout << "result_code[out, prepare] = " << result_code << std::endl;
		std::cout << std::endl;
	}).share();

	if (vm.count("pack"))
	{
		if (!is_install)
		{
			builder_task.then([cmake_path, &result_code]() mutable //async::shared_task<void>
			{
				if (result_code)
					return;

				std::cout << "Cmake install ..." << std::endl;
				std::vector<std::string> args;
				args.push_back("--target ");
				args.push_back("install");
				bp::child c(cmake_path, bp::args(args));
				if (c.joinable())
					c.join();
				result_code = c.exit_code();
				std::cout << "result_code[out, install] = " << result_code << std::endl;
			});
		}

		builder_task.then([cmake_path, &result_code]() mutable
		{
			if (result_code)
				return;			
	
			std::cout << "Cmake pack ..." << std::endl;
			std::vector<std::string> args;
			args.push_back("--target ");
			args.push_back("package");
			bp::child c(cmake_path, bp::args(args));
			result_code = c.exit_code();
			std::cout << "result_code[out, pack] = " << result_code << std::endl;
		});
	}

	if (vm.count("install"))
	{		
		builder_task.then([cmake_path, &result_code]() mutable //async::shared_task<void>
		{
			if (result_code)
				return;
	
			std::cout << "Cmake install ..." << std::endl;
			std::vector<std::string> args;
			args.push_back("--target ");
			args.push_back("install");
			bp::child c(cmake_path, bp::args(args));
			if (c.joinable())
				c.join();
			result_code = c.exit_code();
			std::cout << "result_code[out, install] = " << result_code << std::endl;
		});
		if (!result_code)
			is_install = true;
	}

	builder_task.then([cmake_path, &result_code]() mutable //async::shared_task<void>
	{
		if (result_code)
			return;

		std::cout << "Cmake build ..." << std::endl;
		std::vector<std::string> args;
		args.push_back("--build");
		args.push_back("_builds");
		bp::child c(cmake_path, bp::args(args));
		if (c.joinable())
			c.join();
		result_code = c.exit_code();
		std::cout << "result_code[out, build] = " << result_code << std::endl;
		std::cout << std::endl;
	});
	//
	//builder_task.get();
	//
	//auto task_prepare = async::spawn([cmake_path, config]()-> int
	//{
	//	std::cout << "Cmake prepare ..." << std::endl;
	//	std::vector<std::string> args;
	//	args.push_back("-H.");
	//	args.push_back("-B_builds");
	//	args.push_back("-DCMAKE_INSTALL_PREFIX=_install");
	//	args.push_back(config);
	//	bp::child c(cmake_path, bp::args(args), bp::std_out > stdout, bp::std_err > stderr);
	//	return c.exit_code();		
	//});
	//
	//auto task_build = task_prepare.then([cmake_path](int prepare_result)->int
	//{
	//	if (prepare_result)
	//		return prepare_result;
	//
	//	std::cout << "Cmake build ..." << std::endl;
	//	std::vector<std::string> args;
	//	args.push_back("--build");
	//	args.push_back("_builds");
	//	bp::child c(cmake_path, bp::args(args), bp::std_out > stdout, bp::std_err > stderr);
	//	return c.exit_code();
	//});
	//
	//if (vm.count("install"))
	//{
	//	timeout = vm["timeout"].as<int>();
	//	std::cout << "timeout" << config.c_str() << std::endl;
	//}
	//
	//if (vm.count("pack"))
	//{
	//	timeout = vm["timeout"].as<int>();
	//	std::cout << "timeout" << config.c_str() << std::endl;
	//}
	//
	//if (vm.count("timeout"))
	//{
	//	timeout = vm["timeout"].as<int>();
	//	std::cout << "timeout" << config.c_str() << std::endl;
	//}
	//
	////auto task4 = async::when_all(task_prepare, task_build);
	//
	////auto task_current = task_build;
	//
	//
	//
	//
	//
	//std::cout << "async::spawn" << std::endl;
	//
	////task1.get();
	//
	//std::cout << "task get" << std::endl;
	//
	////auto task1 = async::spawn([]() -> int
	////{
	////	//std::cout << "Task 1 executes asynchronously" << std::endl;
	////	boost::filesystem::path cmake_path = bp::search_path("cmake");
	////	std::cout << "cmake_path = " << cmake_path << std::endl;
	////	bp::system(cmake_path, "-H.", "-B_builds", "-DCMAKE_INSTALL_PREFIX=_install", config, bp::std_out > stdout, bp::std_err > stderr);
	////});
	////
	////auto task2 = task1.then([](int x) -> int
	////{
	////	//std::cout << "Task 1 executes asynchronously" << std::endl;
	////	boost::filesystem::path cmake_path = bp::search_path("cmake");
	////	std::cout << "cmake_path = " << cmake_path << std::endl;
	////	bp::system(cmake_path, "-H.", "-B_builds", "-DCMAKE_INSTALL_PREFIX=_install", config, bp::std_out > stdout, bp::std_err > stderr);
	////});


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
