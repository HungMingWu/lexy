// INPUT:Hello\nWorld   !\n
struct production
{
    static constexpr auto rule = [] {
        auto ws = dsl::whitespace(dsl::ascii::space);
        return dsl::lit<"Hello"> + ws + dsl::lit<"World"> //
               + ws + dsl::exclamation_mark + ws + dsl::eof;
    }();
};
