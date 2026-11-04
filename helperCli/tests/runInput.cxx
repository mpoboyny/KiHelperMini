#include <gtest/gtest.h>
#include "cli-ui.hxx"

#include <string>

// Scripts a sequence of characters as the reader's console input.
// Returns what ui::read_run_input hands to the chat loop, i.e. the line with
// its trailing '\n' kept, or "" for a real EOF.
static std::string read_run_input_with_keys(const std::string & keys)
{
    size_t pos = 0;
    run_input_source = [&]() -> char32_t {
        if (pos >= keys.size()) {
            return WEOF;
        }
        return static_cast<char32_t>(static_cast<unsigned char>(keys[pos++]));
    };
    std::string result = ui::read_run_input(false);
    run_input_source = nullptr;
    return result;
}

// Tab ends the command on the same line; typing after it keeps the line open
// and Enter submits the whole thing.
TEST(RunInput, TabEndsCommandThenEnterSubmits)
{
    EXPECT_EQ(read_run_input_with_keys("/run\tdir\r"), "/run dir\n");
}

TEST(RunInput, TabEndsHashCommandThenEnterSubmits)
{
    EXPECT_EQ(read_run_input_with_keys("/#\tdir\r"), "/# dir\n");
}

// Tab does not submit on its own: no Enter means the read is still pending,
// so a following WEOF returns whatever was typed so far.
TEST(RunInput, TabAloneDoesNotSubmit)
{
    // "/run" + Tab leaves the reader waiting; the injected input then ends
    // (WEOF) with a non-empty line, which is submitted with a newline.
    EXPECT_EQ(read_run_input_with_keys("/run\t"), "/run \n");
}

TEST(RunInput, EnterSubmitsRunCommand)
{
    EXPECT_EQ(read_run_input_with_keys("/run dir\r"), "/run dir\n");
}

TEST(RunInput, EnterSubmitsHashCommand)
{
    EXPECT_EQ(read_run_input_with_keys("/# dir\r"), "/# dir\n");
}

TEST(RunInput, TabDoesNotDoubleSpace)
{
    // The line already ends in a space, so Tab must not add another.
    EXPECT_EQ(read_run_input_with_keys("/run \t\r"), "/run \n");
}

TEST(RunInput, EnterOnEmptyLineIsSkippableLineNotEOF)
{
    // Must stay a line ("\n"), not collapse into the EOF marker ("");
    // the chat loop breaks on "" and skips "\n".
    EXPECT_EQ(read_run_input_with_keys("\r"), "\n");
}

TEST(RunInput, CtrlDEmptyLineIsEOF)
{
    EXPECT_EQ(read_run_input_with_keys("\x04"), "");
}

TEST(RunInput, TabOnPartialRunIsCompletionNotSubmit)
{
    // "/ru" is not yet a /run command: Tab completes it, then Enter submits.
    EXPECT_EQ(read_run_input_with_keys("/ru\t\r"), "/run \n");
}

TEST(RunInput, TabOnSlashAloneKeepsLine)
{
    // No completion candidate for "/": Tab does nothing, Enter submits "/".
    EXPECT_EQ(read_run_input_with_keys("/\t\r"), "/\n");
}
