#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <initializer_list>
#include <algorithm>
#include <functional>

namespace stdarg {
    template<typename CharT>
    class basic_args {
    public:
        using const_pointer          = const CharT*;
        using string                 = std::basic_string<CharT>;
        using vector                 = std::vector<string>;

        using iterator               = vector::              iterator;
        using const_iterator         = vector::        const_iterator;
        using reverse_iterator       = vector::      reverse_iterator;
        using const_reverse_iterator = vector::const_reverse_iterator;

    protected:
        vector args;

    public:
        basic_args(size_t        argc,
                   const_pointer argv[]) {
            for (size_t i = 0; i < argc; i++) args.push_back(argv[i]);
        }

        [[nodiscard]] constexpr               iterator   begin()       noexcept { return args.  begin(); }
        [[nodiscard]] constexpr         const_iterator   begin() const noexcept { return args. cbegin(); }
        [[nodiscard]] constexpr               iterator     end()       noexcept { return args.    end(); }
        [[nodiscard]] constexpr         const_iterator     end() const noexcept { return args.   cend(); }
        [[nodiscard]] constexpr       reverse_iterator  rbegin()       noexcept { return args. rbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator  rbegin() const noexcept { return args.crbegin(); }
        [[nodiscard]] constexpr       reverse_iterator    rend()       noexcept { return args.   rend(); }
        [[nodiscard]] constexpr const_reverse_iterator    rend() const noexcept { return args.  crend(); }
        [[nodiscard]] constexpr         const_iterator  cbegin() const noexcept { return args. cbegin(); }
        [[nodiscard]] constexpr         const_iterator    cend() const noexcept { return args.   cend(); }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return args.crbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator   crend() const noexcept { return args.  crend(); }
    };

    using args = basic_args<char>;

    template<typename CharT>
    class basic_option_impl {
    public:
        using const_pointer    = const CharT*;
        using string           = std::basic_string<CharT>;
        using ostream          = std::basic_ostream<CharT>;
        using vector           = std::vector<string>;
        using initializer_list = std::initializer_list<string>;
        using args_type        = basic_args<CharT>;

        static constexpr size_t alias_string_length = 32;

    protected:
        vector aliases;
        string description;

        args_type::iterator find_in_args(args_type args) {
            return std::find_first_of(args.begin(), args.end(), aliases.begin(), aliases.end());
        }

        bool contained_in_args(args_type args) {
            return find_in_args(args) != args.end();
        }

    public:
        basic_option_impl(initializer_list aliases,
                          string           description) :
            aliases(aliases),
            description(description) { }

        virtual bool operator()(args_type) = 0;

        friend ostream& operator<<(ostream& lhs, basic_option_impl& rhs) {
            if (rhs.aliases.size() == 0) return lhs;

            string aliases_string = rhs.aliases[0];

            for (size_t i = 1; i < rhs.aliases.size(); i++) {
                string str = aliases_string + ", " + rhs.aliases[i];

                if (str.size() > alias_string_length) break;
                aliases_string = str;
            }

            return lhs << "    " << aliases_string << std::setw(alias_string_length - aliases_string.size() + 1) << std::setfill(' ') << ' ' << rhs.description << std::endl;
        }
    };

    template<typename CharT, typename FunctionT>
    class basic_option : public basic_option_impl<CharT> {
    public:
        using option_impl      = basic_option_impl<CharT>;
        using const_pointer    = option_impl::const_pointer;
        using initializer_list = option_impl::initializer_list;
        using string           = option_impl::string;
        using args_type        = option_impl::args_type;

        using      function    = std::function<FunctionT>;
        using move_function    = std::function<FunctionT>&&;

    private:
        function option_handler;

    public:
        basic_option(initializer_list aliases,
                     string           description,
                     move_function    option_handler) :
            option_impl(aliases, description),
            option_handler(option_handler) { }

        bool operator()(args_type args) {
            if (option_impl::contained_in_args(args))
                option_handler();

            return true;
        }
    };

    template<typename FunctionT>
    using option = basic_option<char, FunctionT>;

    template<typename CharT, typename ValueT>
    class basic_argument : public basic_option_impl<CharT> {
    private:
        using option_impl      = basic_option_impl<CharT>;
        using const_pointer    = option_impl::const_pointer;
        using initializer_list = option_impl::initializer_list;
        using string           = option_impl::string;
        using args_type        = option_impl::args_type;

        using reference_type   = ValueT&;
        using pointer          = ValueT*;
        using isstream         = std::basic_istringstream<CharT>;
        using osstream         = std::basic_ostringstream<CharT>;

        string  parameter_name;
        pointer ptr;

    public:
        basic_argument(initializer_list aliases,
                       string           description,
                       string           parameter_name,
                       reference_type   reference) :
            option_impl(aliases, description + " (default: " + toString(reference) + ")"),
            parameter_name(parameter_name),
            ptr(&reference) { }

        basic_argument(initializer_list aliases,
                       string           description,
                       string           parameter_name,
                       pointer          ptr) :
            option_impl(aliases, description + " (default: " + toString(*ptr) + ")"),
            parameter_name(parameter_name),
            ptr(ptr) { }

        bool operator()(args_type args) {
            auto iterator = option_impl::find_in_args(args);

            if (iterator != args.end()) {
                if (++iterator != args.end())
                    return false;

                isstream stream { *iterator };
                stream >> *ptr;
            }

            return true;
        }

        string toString(ValueT value) {
            osstream stream { };

            stream << value;

            return stream.str();
        }
    };

    template<typename ValueT>
    using argument = basic_argument<char, ValueT>;

    template<typename CharT>
    class basic_arg_parser {
    public:
        using args_type      = basic_args<CharT>;
        using option_impl    = std::shared_ptr<basic_option_impl<CharT>>;
        using options_vector = std::vector<std::shared_ptr<basic_option_impl<CharT>>>;
        using string         = std::basic_string<CharT>;

    private:
        options_vector options;
        args_type      args;
        string         description;

    public:
        basic_arg_parser(args_type args,
                         string    description) :
            options(),
            args(args),
            description(description) { }

        template<typename FunctionT>
        basic_arg_parser& operator<<(basic_option<CharT, FunctionT>&& option) {
            options.push_back((option_impl)std::make_shared<basic_option<CharT, FunctionT>>(option));
            return *this;
        }

        template<typename ValueT>
        basic_arg_parser& operator<<(basic_argument<CharT, ValueT>&& argument) {
            options.push_back((option_impl)std::make_shared<basic_argument<CharT, ValueT>>(argument));
            return *this;
        }

        static void help(const basic_arg_parser& parser) {
            std::cout << parser.description << std::endl
                      << std::endl;

            for (auto& option : parser.options)
                std::cout << option;

            exit(EXIT_SUCCESS);
        }

        void operator()() {
            for (auto& option : options) {
                if (!(*option)(args)) {
                    help(*this);
                    std::abort();
                }
            }
        }

        std::string& getDescription() { return description; }
    };

    using arg_parser = basic_arg_parser<char>;
}
