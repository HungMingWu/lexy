// INPUT:Hello World   \n
struct production
{
    struct trailing_spaces
    {
        static constexpr auto name = "trailing spaces";
    };

    static constexpr auto rule
        = dsl::lit<"Hello World">
          + dsl::peek_not(dsl::while_one(dsl::ascii::space)).error<trailing_spaces> + dsl::eof;
};
