// INPUT:sTrUcT
struct production
{
    static constexpr auto rule = [] {
        // Define the general identifier syntax.
        auto head = dsl::ascii::alpha_underscore;
        auto tail = dsl::ascii::alpha_digit_underscore;
        auto id   = dsl::identifier(head, tail);

        // Define some case insensitive keywords.
        auto kw_int    = dsl::ascii::case_folding(dsl::keyword<"int">(id));
        auto kw_struct = dsl::ascii::case_folding(dsl::keyword<"struct">(id));
        // ...

        // Parse an identifier that is not a keyword.
        return id.reserve(kw_int, kw_struct);
    }();
};
