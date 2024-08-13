// INPUT:ab
struct production
{
    static constexpr auto rule = dsl::lit<"a"> >> dsl::lit<"bc">  // a, then bc
                                 | dsl::lit<"a"> >> dsl::lit<"b"> // a, then b
                                 | dsl::lit<"bc"> | dsl::lit<"b">;
};
