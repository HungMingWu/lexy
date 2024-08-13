// INPUT:atom
struct production
{
    static constexpr auto atext
        = define_char_class<"atext">(
                          dsl::ascii::alpha / dsl::ascii::digit / dsl::lit<"!"> / dsl::lit<"#">
                              / dsl::lit<"$"> / dsl::lit<"%"> / dsl::lit<"&"> / dsl::lit<"'">
                              / dsl::lit<"*"> / dsl::lit<"+"> / dsl::lit<"-"> / dsl::lit<"/">
                              / dsl::lit<"="> / dsl::lit<"?"> / dsl::lit<"^"> / dsl::lit<"_">
                              / dsl::lit<"`"> / dsl::lit<"{"> / dsl::lit<"|"> / dsl::lit<"}">);

    static constexpr auto rule = dsl::identifier(atext);
};
