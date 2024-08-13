// INPUT:Hello abx!
struct name
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha);
};

struct alphabet
{
    static constexpr auto rule
        // Just something stupid, so we can see a backtrack.
        = dsl::peek(dsl::lit<"abc">) >> dsl::lit<"abcdefg">;
};

struct number
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::digit);
};

struct object
{
    struct unexpected
    {
        static constexpr auto name = "unexpected";
    };

    static constexpr auto rule
        = dsl::p<alphabet> | dsl::p<name> | dsl::p<number>
         // Issue an error, but recover.
         | dsl::try_(dsl::error<unexpected>);
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto greeting = dsl::lit<"Hello">;
        return greeting + dsl::debug<"finished greeting"> //
               + dsl::p<object> + dsl::exclamation_mark;
    }();
};
