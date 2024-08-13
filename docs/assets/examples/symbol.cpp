#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    // Map names of the entities to their replacement value.
    static constexpr auto entities = lexy::symbol_table<char>
                                         .map<"quot">('"')
                                         .map<"amp">('&')
                                         .map<"apos">('\'')
                                         .map<"lt">('<')
                                         .map<"gt">('>');

    static constexpr auto rule = [] {
        auto name      = dsl::identifier(dsl::ascii::alpha);
        auto reference = dsl::symbol<entities>(name);
        return dsl::lit_c<'&'> >> reference + dsl::lit_c<';'>;
    }();

    static constexpr auto value = lexy::forward<char>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The replacement is: '%c'\n", result.value());
}

