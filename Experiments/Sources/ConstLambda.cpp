#include <stdio.h>

class ConstLambda
{
private:
    int _val;

public:
    constexpr ConstLambda(int val)
        : _val{val}
    {}

    inline void operator()()
    {
        const auto* const_this = this;
        
        [const_this]()
        {
            printf("%d\n", const_this->_val);
        }();
    }
};

int main()
{
    ConstLambda{5}();
}