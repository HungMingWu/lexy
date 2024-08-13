// INPUT:Hello /* C comment */\nWorld   !
struct production
{
    // Note that an unterminated C comment will raise an error.
    static constexpr auto whitespace
        = dsl::ascii::space | dsl::lit<"/*"> >> dsl::until(dsl::lit<"*/">);

    static constexpr auto rule //
        = dsl::lit<"Hello"> + dsl::lit<"World"> + dsl::exclamation_mark;
};
