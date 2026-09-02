#include <nlohmann/json.hpp>
#include <snap/snap.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

static std::string json_to_tree(const nlohmann::json& json, std::size_t depth = 0ULL);
static std::string repeat(std::size_t count, std::string str);

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
            .help("コンパクト表示でプリントします"))
        .arg(Arg<bool>{"tree"}
            .longer()
            .shorter()
            .help("ツリー形式でプリントします"));

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
    if (*result.at("tree"))
        std::cout << "root\n" << json_to_tree(j) << std::endl;
    else
        std::cout << j.dump(get_indent_or_def(result, default_indent)) << std::endl;
    return 0;
}

static std::string json_to_tree(const nlohmann::json& json, std::size_t depth)
{
    std::string str{};
    for (auto& [key, val] : json.items())
    {
        str += repeat(depth, "│  ")
            + "├─ "
            + key
            + ": "
            + val.type_name()
            + "\n";
        if (val.is_object() || val.is_array())
            str += json_to_tree(val, depth + 1);
    }
    return str;
}

static std::string repeat(std::size_t count, std::string str)
{
    std::string result{};
    for (std::size_t i = 0; i < count; i++)
    {
        result += str;
    }
    return result;
}

int get_indent_or_def(const snap::App::ParseResult& result, int def)
{
    if (*result.at("compact"))
        return -1;
    if (*result.at("indent"))
        return result.at("indent")->values<int>().at(0);
    return def;
}