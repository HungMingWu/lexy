// INPUT:Hello\nWorld   !
struct production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule                  //
        = dsl::lit<"Hello"> + dsl::lit<"World"> //
          + dsl::exclamation_mark + dsl::eof;
};
