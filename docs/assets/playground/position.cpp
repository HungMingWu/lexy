// INPUT:function foo(...)\n{\n  ...\n}
struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto id          = dsl::identifier(dsl::ascii::alpha);
        auto kw_function = dsl::keyword<"function">(id);

        auto arguments = dsl::parenthesized(dsl::lit<"...">);
        auto body      = dsl::curly_bracketed(dsl::lit<"...">);

        // The position of a function is the first character of the name.
        return kw_function + dsl::position + id + arguments + body;
    }();
};
