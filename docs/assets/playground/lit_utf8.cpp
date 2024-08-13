// INPUT:ä
struct production
{
    // The string literal contains UTF-8 text,
    // which means the input needs to be UTF-8 encoded as well.
    //
    // WARNING: This will only match if both agree on a normalization for 'ä'!
    static constexpr auto rule = dsl::lit<u8"ä">;
};
