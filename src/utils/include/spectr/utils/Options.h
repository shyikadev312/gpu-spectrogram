#pragma once

#include <initializer_list>
#include <vector>
#include <string>

namespace spectr {
    namespace utils
    {
        class BasicOption {
        private:
            std::vector<std::string> shortOptions;
            std::vector<std::string> longOptions;
            std::string              description;

        public:
            BasicOption(std::initializer_list<std::string> shortOptions,
                        std::initializer_list<std::string> longOptions,
                        std::string description) :
                        shortOptions(shortOptions),
                        longOptions(longOptions),
                        description(description) { }
        };

        template<typename T>
        class VarOption : BasicOption {
        private:
            using value_type = T;
            using reference  = T&;

            reference ref;

        public:
            VarOption(std::initializer_list<std::string> shortOptions,
                      std::initializer_list<std::string> longOptions,
                      std::string description,
                      reference ref) :
                      BasicOption(shortOptions, longOptions, description),
                      ref(ref) { }
        };
    }
}
