#include <cstdint>
#include <cstdio>
#include <string>

#if __cplusplus < 202002L
#error "Please use at least C++20 or newest version. (officially this will work only in C++23 and when __cpp_constexpr >= 202211L)"
#else
template <size_t kNodesCount, char kMinChar, char kMaxChar>
class StringSwitch {
    static_assert(__cplusplus >= 202002L);

    template <size_t Size>
    struct TrieNode {
        uint32_t children[Size] = {};
        uint16_t string_index = 0;
        bool terminal = false;
    };

    static_assert(kMinChar <= kMaxChar);
    using Node = TrieNode<size_t(kMaxChar - kMinChar) + 1>;
    Node nodes[kNodesCount] = {};

    consteval void AddPattern(size_t, size_t) noexcept {}

    template <class... Args>
    consteval void AddPattern(size_t first_free_node_index, size_t string_index,
                              const char* string, Args... args) noexcept {
        size_t current_node = 0;
        for (int32_t c; (c = uint8_t(*string)) != '\0'; string++) {
            size_t index = size_t(c - kMinChar);
            size_t child_index = nodes[current_node].children[index];
            if (child_index != 0) {
                current_node = child_index;
            } else {
                nodes[current_node].children[index] =
                    uint16_t(first_free_node_index);
                current_node = first_free_node_index;
                first_free_node_index++;
            }
        }

        nodes[current_node].terminal = true;
        nodes[current_node].string_index = uint16_t(string_index);
        AddPattern(first_free_node_index, string_index + 1, args...);
    }

   public:
    static constexpr uint32_t kDefaultSwitch = uint32_t(-1);

    template <class... Args>
    consteval StringSwitch(Args... args) noexcept {
        // 0 is root index, start from 1
        AddPattern(size_t(1), size_t(0), args...);
    }

    constexpr uint32_t Switch(const char* string) const noexcept {
        size_t current_node = 0;

        if (string == nullptr) {
            return kDefaultSwitch;
        }

        for (int32_t c; (c = uint8_t(*string)) != '\0'; string++) {
            size_t index = size_t(c - kMinChar);
            if (index > size_t(kMaxChar - kMinChar)) {
                return kDefaultSwitch;
            }

            size_t next_node = nodes[current_node].children[index];
            if (next_node != 0) {
                current_node = next_node;
            } else {
                return kDefaultSwitch;
            }
        }

        if (nodes[current_node].terminal) {
            return nodes[current_node].string_index;
        }

        return kDefaultSwitch;
    }
};
#endif

int main() {
    static_assert(__cplusplus >= 202002L);

    using StrSwitch = StringSwitch<18, 'a', 'z'>;
    static constexpr StrSwitch sw("abc", "def", "ghij", "foo", "bar", "baz");
    static_assert(sw.Switch("abc") == 0);
    static_assert(sw.Switch("def") == 1);
    static_assert(sw.Switch("ghij") == 2);
    static_assert(sw.Switch("foo") == 3);
    static_assert(sw.Switch("bar") == 4);
    static_assert(sw.Switch("baz") == 5);
    static_assert(sw.Switch(nullptr) == StrSwitch::kDefaultSwitch);
    static_assert(sw.Switch("") == StrSwitch::kDefaultSwitch);
    static_assert(sw.Switch("a") == StrSwitch::kDefaultSwitch);
    static_assert(sw.Switch("A") == StrSwitch::kDefaultSwitch);
    static_assert(sw.Switch("de") == StrSwitch::kDefaultSwitch);
    static_assert(sw.Switch("ghi") == StrSwitch::kDefaultSwitch);
    static_assert(sw.Switch("not_in_switch") == StrSwitch::kDefaultSwitch);

    char input[16];
    scanf("%15s", input);
    const char* ans;
    switch (sw.Switch(input)) {
        case 0:
            ans = "abc!";
            break;
        case 1:
            ans = "def!";
            break;
        case 2:
            ans = "ghij!";
            break;
        case 3:
            ans = "foo!";
            break;
        case 4:
            ans = "bar!";
            break;
        case 5:
            ans = "baz!";
            break;
        case StrSwitch::kDefaultSwitch:
            ans = "not in switch!";
            break;
        default:
            ans = "switch failure!";
            break;
    }
    puts(ans);
}
