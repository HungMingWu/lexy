// INPUT:state;
struct production
{
    static constexpr auto rule = [] {
        auto terminator = dsl::terminator(dsl::semicolon);
        return terminator.try_(dsl::lit<"statement">);
    }();
};
