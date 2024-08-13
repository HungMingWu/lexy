// INPUT:Hallo
struct production
{
    static constexpr auto rule
        // Input should be empty if greeting isn't known.
        = dsl::lit<"Hello"> | dsl::lit<"Hi"> | dsl::else_ >> dsl::eof;
};
