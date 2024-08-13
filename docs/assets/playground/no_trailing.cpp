// INPUT:Hello World!
struct production
{
    static constexpr auto rule = dsl::lit<"Hello"> + dsl::eof;
};

