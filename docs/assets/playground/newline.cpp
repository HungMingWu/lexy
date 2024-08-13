// INPUT:Hello\n
struct production
{
    static constexpr auto rule = dsl::lit<"Hello"> + dsl::newline;
};

