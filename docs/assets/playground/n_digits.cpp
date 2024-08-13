// INPUT:\xABCDEF
struct production
{
    static constexpr auto rule = dsl::lit<"\\x"> + dsl::n_digits<2, dsl::hex>;
};
