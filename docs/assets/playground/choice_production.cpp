// INPUT:type Id = Integer;
constexpr auto id          = dsl::identifier(dsl::ascii::alpha);
constexpr auto kw_function = dsl::keyword<"function">(id);
constexpr auto kw_type     = dsl::keyword<"type">(id);

struct function_decl
{
    static constexpr auto rule = [] {
        auto arguments = dsl::parenthesized(dsl::lit<"...">);
        auto body      = dsl::curly_bracketed(dsl::lit<"...">);

        return kw_function >> id + arguments + body;
    }();
};

struct type_decl
{
    static constexpr auto rule //
        = kw_type >> id + dsl::lit_c<'='> + id + dsl::semicolon;
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule       = dsl::p<function_decl> | dsl::p<type_decl>;
};

