// INPUT:(abc,12,abc123,123)
struct production
{
    static constexpr auto rule = [] {
        auto item = dsl::lit<"abc"> | dsl::lit<"123">;

        return dsl::parenthesized.list(item, dsl::sep(dsl::comma));
    }();
};
