#include <nlohmann/json.hpp>
#include <snap/snap.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

int get_indent_or_def(const snap::App::ParseResult& result, int def);

int main( int argc, char* argv[] ){
    using namespace snap;
	using json = nlohmann::json;

    const int default_indent = 4;

    /* App Info */
    App app = App{ "myjson" }
        .about("JSONファイルを読み込んで成型・表示するCLIツール")
        .version("1.1")
        .author("Azkey")
        .arg(Arg<std::filesystem::path>{"FILE"})
        .arg(Arg<int>{"indent"}
            .longer()
            .shorter()
            .entry("INDENT")
            .help("インデント幅を整数値で指定します"))
        .arg(Arg<bool>{"compact"}
            .longer()
            .shorter()
            .help("コンパクト表示でプリントします"));

    auto result = app.parse(argc, argv);

    /* Open File */
    auto path = result.at("FILE")->values<std::filesystem::path>().at(0);
    std::ifstream ifs{ path };
    if (!ifs.is_open())
    {
        std::cerr << "File cannot open: " << path << std::endl;
        return 1;
    }

    /* ifstream to json */
    json j;
    try {
        ifs >> j;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    /* print */
    std::cout << j.dump(get_indent_or_def(result, default_indent)) << std::endl;
    return 0;
}

int get_indent_or_def(const snap::App::ParseResult& result, int def)
{
    if (*result.at("compact"))
        return -1;
    if (*result.at("indent"))
        return result.at("indent")->values<int>().at(0);
    return def;
}