// INPUT:Hallo
struct production
{
    struct unknown_greeting
    {
        static constexpr auto name = "unknown greeting";
    };

    static constexpr auto rule
        // Generate a custom error for an unknown greeting.
        = dsl::lit<"Hello"> | dsl::lit<"Hi">
          | dsl::error<unknown_greeting>(dsl::while_(dsl::ascii::alpha));
};
