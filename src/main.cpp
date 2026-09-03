#include <nlohmann/json.hpp>
#include <snap/snap.hpp>

#include <charconv>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include <unordered_map>

static const nlohmann::json& find(const nlohmann::json& json, std::string_view key);
static const nlohmann::json&
    universal_at(const nlohmann::json& json, std::string_view key);
static std::size_t sv_to_size(std::string_view sv);

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
        .version("1.3")
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
            .help("ツリー形式でプリントします"))
        .arg(Arg<std::string>{"find"}
            .longer()
            .shorter()
            .entry("KEY")
            .help("キーに対応する値を取得します"));

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

    const json* print_target = &j;
    if (*result.at("find"))
    {
        auto key = result.at("find")->values<std::string>().at(0);
        try {
            print_target = &find(j, key);
            std::cout << key << " = ";
        }
        catch (const std::logic_error& e) {
            std::cerr << e.what() << ": " << key << std::endl;
            return 1;
        }
    }

    /* print */
    if (*result.at("tree"))
        std::cout << "root\n" << json_to_tree(*print_target) << std::endl;
    else
        std::cout << print_target->dump(
            get_indent_or_def(result, default_indent)
        ) << std::endl;
    return 0;
}

static const nlohmann::json& find(const nlohmann::json& json, std::string_view key)
{
    auto pos = key.find_first_of('.');
    if (pos == std::string_view::npos)
        return universal_at(json, key);
    std::string_view first = key.substr(0, pos);
    std::string_view second = key.substr(pos + 1);
    return find(universal_at(json, first), second);
}

static const nlohmann::json&
universal_at(const nlohmann::json& json, std::string_view key)
{
    try {
        return json.is_array() ? json.at(sv_to_size(key)) : json.at(key);
    }
    catch (const nlohmann::json::out_of_range& e) {
        throw std::out_of_range("Failed to find: " + std::string{ e.what() });
    }
}

static std::size_t sv_to_size(std::string_view sv)
{
    std::size_t s{};
    auto [ptr, ec] = std::from_chars(
        sv.data(),
        sv.data() + sv.size(),
        s
    );
    if (ec != std::errc{} || ptr != sv.data() + sv.size())
        throw std::invalid_argument("Invalid key");

    return s;
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