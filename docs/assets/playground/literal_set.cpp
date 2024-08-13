// INPUT:abc
struct production
{
    static constexpr auto rule //
        = dsl::literal_set(dsl::lit<"a">, dsl::lit<"abc">, dsl::lit<"bc">);
};
