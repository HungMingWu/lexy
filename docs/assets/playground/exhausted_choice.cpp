// INPUT:Hey
struct production
{
    static constexpr auto rule = dsl::lit<"Hello"> | dsl::lit<"Hi">;
};
