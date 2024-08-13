// INPUT:ab
struct production
{
    static constexpr auto rule = dsl::lit<"a"> | dsl::lit<"ab">;
};
