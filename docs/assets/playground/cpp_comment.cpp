// INPUT:// This is a comment.
struct production
{
    static constexpr auto rule = dsl::lit<"//"> + dsl::until(dsl::newline).or_eof();
};
