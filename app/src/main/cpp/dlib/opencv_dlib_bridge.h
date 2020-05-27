#include <string>
#include <sstream>

using namespace std;
namespace std{
    template <typename T> std::string to_string(const T& n)
    {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }

    template <typename T> T round(T v)
    {
        return (v>0)?(v+0.5):(v-0.5);
    }
}