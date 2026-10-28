#include <Serialize/ToString.hpp>

int main() 
{
    for (auto val = 1.; val < 11.; ++val)
    {
        for (auto idx = -323; idx != 308; ++idx)
        {
            auto v = hsd::math::pow(val, static_cast<hsd::f32>(idx));
            auto buf = hsd::to_pseudo_string_exp(v);
            printf("%.*s\n", (int)buf.size(), buf.begin());
        }
    }
}