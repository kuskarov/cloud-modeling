#include "cli.h"

#include <cerrno>
#include <cstring>
#include <regex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "replxx.hxx"

using namespace replxx;

int
utf8str_codepoint_len(char const* s, int utf8len)
{
    int codepointLen = 0;
    unsigned char m4 = 128 + 64 + 32 + 16;
    unsigned char m3 = 128 + 64 + 32;
    unsigned char m2 = 128 + 64;
    for (int i = 0; i < utf8len; ++i, ++codepointLen) {
        char c = s[i];
        if ((c & m4) == m4) {
            i += 3;
        } else if ((c & m3) == m3) {
            i += 2;
        } else if ((c & m2) == m2) {
            i += 1;
        }
    }
    return (codepointLen);
}

int
context_len(char const* prefix)
{
    char const wb[] = " \t\n\r\v\f-=+*&^%$#@!,./?<>;:`~'\"[]{}()\\|";
    int i = (int)strlen(prefix) - 1;
    int cl = 0;
    while (i >= 0) {
        if (strchr(wb, prefix[i]) != NULL) {
            break;
        }
        ++cl;
        --i;
    }
    return (cl);
}

Replxx::completions_t
hook_completion(std::string const& context, int& contextLen,
                std::vector<std::string> const& examples)
{
    Replxx::completions_t completions;
    int utf8ContextLen(context_len(context.c_str()));
    int prefixLen(static_cast<int>(context.length()) - utf8ContextLen);
    if ((prefixLen > 0) && (context[prefixLen - 1] == '\\')) {
        --prefixLen;
        ++utf8ContextLen;
    }
    contextLen =
        utf8str_codepoint_len(context.c_str() + prefixLen, utf8ContextLen);

    std::string prefix{context.substr(prefixLen)};
    if (prefix == "\\pi") {
        completions.push_back("π");
    } else {
        for (auto const& e : examples) {
            if (e.compare(0, prefix.size(), prefix) == 0) {
                Replxx::Color c(Replxx::Color::DEFAULT);
                if (e.find("brightred") != std::string::npos) {
                    c = Replxx::Color::BRIGHTRED;
                } else if (e.find("red") != std::string::npos) {
                    c = Replxx::Color::RED;
                }
                completions.emplace_back(e.c_str(), c);
            }
        }
    }

    return completions;
}

Replxx::hints_t
hook_hint(std::string const& context, int& contextLen, Replxx::Color& color,
          std::vector<std::string> const& examples)
{
    Replxx::hints_t hints;

    // only show hint if prefix is at least 'n' chars long
    // or if prefix begins with a specific character

    int utf8ContextLen(context_len(context.c_str()));
    int prefixLen(static_cast<int>(context.length()) - utf8ContextLen);
    contextLen =
        utf8str_codepoint_len(context.c_str() + prefixLen, utf8ContextLen);
    std::string prefix{context.substr(prefixLen)};

    if (prefix.size() >= 2 || (!prefix.empty() && prefix.at(0) == '.')) {
        for (auto const& e : examples) {
            if (e.compare(0, prefix.size(), prefix) == 0) {
                hints.emplace_back(e.c_str());
            }
        }
    }

    // set hint color to green if single match found
    if (hints.size() == 1) {
        color = Replxx::Color::GREEN;
    }

    return hints;
}

void
hook_color(
    std::string const& context, Replxx::colors_t& colors,
    std::vector<std::pair<std::string, Replxx::Color>> const& regex_color)
{
    // highlight matching regex sequences
    for (auto const& e : regex_color) {
        size_t pos{0};
        std::string str = context;
        std::smatch match;

        while (std::regex_search(str, match, std::regex(e.first))) {
            std::string c{match[0]};
            std::string prefix(match.prefix().str());
            pos += utf8str_codepoint_len(prefix.c_str(),
                                         static_cast<int>(prefix.length()));
            int len(
                utf8str_codepoint_len(c.c_str(), static_cast<int>(c.length())));

            for (int i = 0; i < len; ++i) {
                colors.at(pos + i) = e.second;
            }

            pos += len;
            str = match.suffix();
        }
    }
}

Replxx::ACTION_RESULT
message(Replxx& replxx, std::string s, char32_t)
{
    replxx.invoke(Replxx::ACTION::CLEAR_SELF, 0);
    replxx.print("%s\n", s.c_str());
    replxx.invoke(Replxx::ACTION::REPAINT, 0);
    return (Replxx::ACTION_RESULT::CONTINUE);
}

bool
sim::client::CLI::Setup()
{
    try {
        rx_.install_window_change_handler();

        // load the history file if it exists
        rx_.history_load(history_file);

        // set the max history size
        rx_.set_max_history_size(128);

        // set the max number of hint rows to show
        rx_.set_max_hint_rows(3);

        // set the callbacks
        using namespace std::placeholders;
        rx_.set_completion_callback(
            std::bind(&hook_completion, _1, _2, cref(examples)));
        rx_.set_highlighter_callback(
            std::bind(&hook_color, _1, _2, cref(regex_color)));
        rx_.set_hint_callback(
            std::bind(&hook_hint, _1, _2, _3, cref(examples)));

        // other api calls
        rx_.set_word_break_characters(" \t.,-%!;:=*~^'\"/?<>|[](){}");
        rx_.set_completion_count_cutoff(128);
        rx_.set_double_tab_completion(false);
        rx_.set_complete_on_empty(true);
        rx_.set_beep_on_ambiguous_completion(false);
        rx_.set_no_color(false);

        BindKeys();

        return true;
    } catch (...) {
        return false;
    }
}

void
sim::client::CLI::RunLoop()
{
    // main repl loop
    for (;;) {
        // display the prompt and retrieve input from the user
        char const* cinput{nullptr};

        do {
            cinput = rx_.input(prompt);
        } while ((cinput == nullptr) && (errno == EAGAIN));

        if (cinput == nullptr) {
            break;
        }

        // change cinput into a std::string
        // easier to manipulate
        std::string input{cinput};

        if (input.empty()) {
            continue;
        } else {
            rx_.history_add(input);

            // pass to RPC-Client
            process_callback(input);
        }
    }

    // save the history
    rx_.history_sync(history_file);
}

void
sim::client::CLI::BindKeys()
{
    using namespace std::placeholders;

    rx_.bind_key_internal(Replxx::KEY::BACKSPACE,
                          "delete_character_left_of_cursor");
    rx_.bind_key_internal(Replxx::KEY::DELETE, "delete_character_under_cursor");
    rx_.bind_key_internal(Replxx::KEY::LEFT, "move_cursor_left");
    rx_.bind_key_internal(Replxx::KEY::RIGHT, "move_cursor_right");
    rx_.bind_key_internal(Replxx::KEY::UP, "history_previous");
    rx_.bind_key_internal(Replxx::KEY::DOWN, "history_next");
    rx_.bind_key_internal(Replxx::KEY::PAGE_UP, "history_first");
    rx_.bind_key_internal(Replxx::KEY::PAGE_DOWN, "history_last");
    rx_.bind_key_internal(Replxx::KEY::HOME, "move_cursor_to_begining_of_line");
    rx_.bind_key_internal(Replxx::KEY::END, "move_cursor_to_end_of_line");
    rx_.bind_key_internal(Replxx::KEY::TAB, "complete_line");
    rx_.bind_key_internal(Replxx::KEY::control(Replxx::KEY::LEFT),
                          "move_cursor_one_word_left");
    rx_.bind_key_internal(Replxx::KEY::control(Replxx::KEY::RIGHT),
                          "move_cursor_one_word_right");
    rx_.bind_key_internal(Replxx::KEY::control(Replxx::KEY::UP),
                          "hint_previous");
    rx_.bind_key_internal(Replxx::KEY::control(Replxx::KEY::DOWN), "hint_next");
    rx_.bind_key_internal(Replxx::KEY::control(Replxx::KEY::ENTER),
                          "commit_line");
    rx_.bind_key_internal(Replxx::KEY::control('R'),
                          "history_incremental_search");
    rx_.bind_key_internal(Replxx::KEY::control('W'),
                          "kill_to_begining_of_word");
    rx_.bind_key_internal(Replxx::KEY::control('U'),
                          "kill_to_begining_of_line");
    rx_.bind_key_internal(Replxx::KEY::control('K'), "kill_to_end_of_line");
    rx_.bind_key_internal(Replxx::KEY::control('Y'), "yank");
    rx_.bind_key_internal(Replxx::KEY::control('L'), "clear_screen");
    rx_.bind_key_internal(Replxx::KEY::control('D'), "send_eof");
    rx_.bind_key_internal(Replxx::KEY::control('C'), "abort_line");
    rx_.bind_key_internal(Replxx::KEY::control('T'), "transpose_characters");
#ifndef _WIN32
    rx_.bind_key_internal(Replxx::KEY::control('V'), "verbatim_insert");
    rx_.bind_key_internal(Replxx::KEY::control('Z'), "suspend");
#endif
    rx_.bind_key_internal(Replxx::KEY::meta(Replxx::KEY::BACKSPACE),
                          "kill_to_whitespace_on_left");
    rx_.bind_key_internal(Replxx::KEY::meta('p'),
                          "history_common_prefix_search");
    rx_.bind_key_internal(Replxx::KEY::meta('n'),
                          "history_common_prefix_search");
    rx_.bind_key_internal(Replxx::KEY::meta('d'), "kill_to_end_of_word");
    rx_.bind_key_internal(Replxx::KEY::meta('y'), "yank_cycle");
    rx_.bind_key_internal(Replxx::KEY::meta('u'), "uppercase_word");
    rx_.bind_key_internal(Replxx::KEY::meta('l'), "lowercase_word");
    rx_.bind_key_internal(Replxx::KEY::meta('c'), "capitalize_word");
    rx_.bind_key_internal('a', "insert_character");
    rx_.bind_key_internal(Replxx::KEY::INSERT, "toggle_overwrite_mode");
    rx_.bind_key(Replxx::KEY::F1,
                 std::bind(&message, std::ref(rx_), "<F1>", _1));
    rx_.bind_key(Replxx::KEY::F2,
                 std::bind(&message, std::ref(rx_), "<F2>", _1));
    rx_.bind_key(Replxx::KEY::F3,
                 std::bind(&message, std::ref(rx_), "<F3>", _1));
    rx_.bind_key(Replxx::KEY::F4,
                 std::bind(&message, std::ref(rx_), "<F4>", _1));
    rx_.bind_key(Replxx::KEY::F5,
                 std::bind(&message, std::ref(rx_), "<F5>", _1));
    rx_.bind_key(Replxx::KEY::F6,
                 std::bind(&message, std::ref(rx_), "<F6>", _1));
    rx_.bind_key(Replxx::KEY::F7,
                 std::bind(&message, std::ref(rx_), "<F7>", _1));
    rx_.bind_key(Replxx::KEY::F8,
                 std::bind(&message, std::ref(rx_), "<F8>", _1));
    rx_.bind_key(Replxx::KEY::F9,
                 std::bind(&message, std::ref(rx_), "<F9>", _1));
    rx_.bind_key(Replxx::KEY::F10,
                 std::bind(&message, std::ref(rx_), "<F10>", _1));
    rx_.bind_key(Replxx::KEY::F11,
                 std::bind(&message, std::ref(rx_), "<F11>", _1));
    rx_.bind_key(Replxx::KEY::F12,
                 std::bind(&message, std::ref(rx_), "<F12>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F1),
                 std::bind(&message, std::ref(rx_), "<S-F1>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F2),
                 std::bind(&message, std::ref(rx_), "<S-F2>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F3),
                 std::bind(&message, std::ref(rx_), "<S-F3>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F4),
                 std::bind(&message, std::ref(rx_), "<S-F4>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F5),
                 std::bind(&message, std::ref(rx_), "<S-F5>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F6),
                 std::bind(&message, std::ref(rx_), "<S-F6>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F7),
                 std::bind(&message, std::ref(rx_), "<S-F7>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F8),
                 std::bind(&message, std::ref(rx_), "<S-F8>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F9),
                 std::bind(&message, std::ref(rx_), "<S-F9>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F10),
                 std::bind(&message, std::ref(rx_), "<S-F10>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F11),
                 std::bind(&message, std::ref(rx_), "<S-F11>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::F12),
                 std::bind(&message, std::ref(rx_), "<S-F12>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F1),
                 std::bind(&message, std::ref(rx_), "<C-F1>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F2),
                 std::bind(&message, std::ref(rx_), "<C-F2>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F3),
                 std::bind(&message, std::ref(rx_), "<C-F3>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F4),
                 std::bind(&message, std::ref(rx_), "<C-F4>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F5),
                 std::bind(&message, std::ref(rx_), "<C-F5>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F6),
                 std::bind(&message, std::ref(rx_), "<C-F6>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F7),
                 std::bind(&message, std::ref(rx_), "<C-F7>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F8),
                 std::bind(&message, std::ref(rx_), "<C-F8>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F9),
                 std::bind(&message, std::ref(rx_), "<C-F9>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F10),
                 std::bind(&message, std::ref(rx_), "<C-F10>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F11),
                 std::bind(&message, std::ref(rx_), "<C-F11>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::F12),
                 std::bind(&message, std::ref(rx_), "<C-F12>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::TAB),
                 std::bind(&message, std::ref(rx_), "<S-Tab>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::HOME),
                 std::bind(&message, std::ref(rx_), "<C-Home>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::HOME),
                 std::bind(&message, std::ref(rx_), "<S-Home>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::END),
                 std::bind(&message, std::ref(rx_), "<C-End>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::END),
                 std::bind(&message, std::ref(rx_), "<S-End>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::PAGE_UP),
                 std::bind(&message, std::ref(rx_), "<C-PgUp>", _1));
    rx_.bind_key(Replxx::KEY::control(Replxx::KEY::PAGE_DOWN),
                 std::bind(&message, std::ref(rx_), "<C-PgDn>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::LEFT),
                 std::bind(&message, std::ref(rx_), "<S-Left>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::RIGHT),
                 std::bind(&message, std::ref(rx_), "<S-Right>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::UP),
                 std::bind(&message, std::ref(rx_), "<S-Up>", _1));
    rx_.bind_key(Replxx::KEY::shift(Replxx::KEY::DOWN),
                 std::bind(&message, std::ref(rx_), "<S-Down>", _1));
}
