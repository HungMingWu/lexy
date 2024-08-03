// INPUT:"123abc"
struct production : lexy::scan_production<int>
{
    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        // Parse the initial quote.
        scanner.parse(dsl::lit_c<'"'>);
        if (!scanner)
            return lexy::scan_failed;

        // Parse an integer.
        lexy::scan_result<int> integer;
        scanner.parse(integer, dsl::integer<int>);
        if (!scanner)
            return lexy::scan_failed;

        // Parse the closing quote.
        scanner.parse(dsl::lit_c<'"'>);
        if (!scanner)
        {
            // Recover by discarding everything until a closing quote is found.
            auto recovery = scanner.error_recovery();
            while (!scanner.branch(dsl::lit_c<'"'>))
            {
                if (!scanner.discard(dsl::ascii::character))
                {
                    // We've failed to recover.
                    std::move(recovery).cancel();
                    return lexy::scan_failed;
                }
            }
            std::move(recovery).finish();
        }

        return integer.value();
    }
};
