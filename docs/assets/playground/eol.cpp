// INPUT:Hello
struct production
{
    static constexpr auto rule = dsl::lit<"Hello"> + dsl::eol;
};

