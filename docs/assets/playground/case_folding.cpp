// INPUT:HeLlO wOrLd!
struct production
{
    static constexpr auto rule = dsl::ascii::case_folding(dsl::lit<"hello world!">);
};
