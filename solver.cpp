#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

// This class needs to be improved. Improved constructor...
struct Equation
{
private:
    void _format(std::string& str)
    {
        std::string temp{};

        for (auto c : str)
        {
            if (c >= 'A' && c <= 'Z')
                temp += c;
        }

        str = temp;
    }
public:
    std::string minuend{};
    std::string subtrahend{};
    std::string difference{};

    static unsigned short extra_letters;

    void format()
    {
        _format(subtrahend);

        const auto pos = minuend.find('|');

        if (pos != std::string::npos)
        {
            minuend.erase(0, pos + 2);
            
            extra_letters = minuend.length() - subtrahend.length();

            minuend.erase(minuend.end() - extra_letters, minuend.end());
        }
        else
        {
            _format(minuend);
            extra_letters--;
        }

        _format(difference);
        if (extra_letters > 0 && difference.length() > 0)
            difference.pop_back();
    }
};

unsigned short Equation::extra_letters{ 0 };

struct Info
{
    int base{};
    std::string key{};
    std::vector<Equation> equations{};
    bool is_solve4{ false };
};

Info get_info(std::ifstream& file)
{
    Info info{};
    std::string curline{};
    
    while (std::getline(file, curline))
    {
        static std::string buffer_minuend{};
        static std::string buffer_subtrahend{};

        if (curline.find('=') != std::string::npos)
        {
            // Get difference
            std::getline(file, curline);

            if (curline.find('?') != std::string::npos)
            {
                info.is_solve4 = true;
                continue;
            }

            Equation e{ buffer_minuend, buffer_subtrahend, curline };
            e.format();
            
            info.equations.push_back(e);

            std::cout << "Found equation: '" << e.minuend << "' - '" << e.subtrahend << "' = '" << e.difference << "'\n";
        }
        else if (curline.starts_with("base:"))
        {
            // Ugly, do better
            info.base = std::atoi(std::string{ curline.substr(curline.find(": ") + 2, curline.length()) }.c_str());

            std::cout << "Found base: '" << info.base << "'\n";
        }
        else if (curline.starts_with("letters:"))
        {
            // Ugly, do better
            info.key = std::string{ curline.substr(curline.find(": ") + 2, curline.length()) };

            std::cout << "Found key: '" << info.key << "'\n";
        }

        buffer_minuend = buffer_subtrahend;
        buffer_subtrahend = curline;
    }

    // Improve error message
    if (!info.base || info.key.empty() || info.equations.empty())
        throw std::runtime_error("Information unobtainable");

    return info;
}

// Can this be improved?
int letters_to_int(const std::string& letters, const std::string& key, int base)
{
    int x{};

    for (std::size_t i = 0; i < letters.length(); i++)
    {
        x += key.find(letters.at(i)) * static_cast<int>(std::pow(base, letters.length() - 1 - i));
    }

    return x;
}

void bruteforce(Info& info)
{
    std::cout << "Bruteforcing, please wait...\n";

    do
    {
        // std::cout << "Trying " << info.key << '\n';

        auto attempt = [info]() 
        {
            for (const auto e : info.equations)
            {
                int minuend{ letters_to_int(e.minuend, info.key, info.base) };
                int subtrahend{ letters_to_int(e.subtrahend, info.key, info.base) };
                int difference{ letters_to_int(e.difference, info.key, info.base) };

                if (subtrahend > minuend || difference > minuend || difference > subtrahend)
                    return false;

                if ((minuend - subtrahend) == difference)
                    return true;
            }
            return false;
        };

        auto verify = [info]()
        {
            bool verified = true;
            for (const auto e : info.equations)
            {
                int minuend{ letters_to_int(e.minuend, info.key, info.base) };
                int subtrahend{ letters_to_int(e.subtrahend, info.key, info.base) };
                int difference{ letters_to_int(e.difference, info.key, info.base) };

                if (subtrahend > minuend || difference > minuend || difference > subtrahend)
                    verified = false;

                if ((minuend - subtrahend) != difference)
                    verified = false;
            }
            return verified;
        };

        if (attempt() && verify())
        {
            std::cout << "Solution verified: " << info.key << '\n';
            if (info.is_solve4)
            {
                std::cout << "Solve4 verification: ";
                // TO DO!!!
            }
        }

    } while (std::next_permutation(info.key.begin(), info.key.end()));

    throw std::runtime_error("Permutations exhausted");
}

int main(int argc, char* argv[])
{
    // Parse options (todo)
    char opt{};

    while ((opt = getopt(argc, argv, ":if:fv")) != -1)
    {
        switch (opt)
        {
            case 'v': std::cout << "Verbose mode selected\n"; break;
            case 'f': std::cout << "Filename: " << optarg << '\n'; break;
            default: std::cout << "Unrecognized option\n"; return 1;
        }
    }

    // Open file "puzzle" for reading
    std::ifstream file{ "puzzle" };

    if (!file.is_open())
    {
        std::cerr << "File 'puzzle' not found.\n";
        return 1;
    }

    try
    {
        auto info = get_info(file);
        bruteforce(info);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        // If an exception is thrown does clean up still occur?
    }

    // Cleanup
    file.close();

    return 0;
}
