#include <Base/Concepts.hpp>
#include <vector>

// Each concept accepts the types it describes and rejects the others. The
// checks are compile-time, so a wrong answer fails the build.

struct point
{
    int x;
    int y;
};

struct move_only
{
    move_only(const move_only&) = delete;
    move_only(move_only&&) = default;
    move_only& operator=(const move_only&) = delete;
    move_only& operator=(move_only&&) = default;
};

static_assert(hsd::IsIntegral<int> && !hsd::IsIntegral<float>);
static_assert(hsd::IsFloat<double> && !hsd::IsFloat<int>);
static_assert(hsd::IsSigned<int> && !hsd::IsSigned<unsigned>);
static_assert(hsd::IsUnsigned<unsigned> && !hsd::IsUnsigned<int>);
static_assert(hsd::IsNatural<unsigned> && !hsd::IsNatural<int> && !hsd::IsNatural<double>);
static_assert(hsd::IsNumber<int> && hsd::IsNumber<double> && !hsd::IsNumber<char*>);
static_assert(hsd::IsObject<int> && !hsd::IsObject<int&> && !hsd::IsObject<void()>);
static_assert(hsd::IsVoid<void> && !hsd::IsVoid<int>);
static_assert(hsd::IsReference<int&> && hsd::IsReference<int&&> && !hsd::IsReference<int>);
static_assert(hsd::IsRvalueRef<int&&> && !hsd::IsRvalueRef<int&>);
static_assert(hsd::IsLvalueRef<int&> && !hsd::IsLvalueRef<int&&>);
static_assert(hsd::IsConst<const int> && !hsd::IsConst<int>);
static_assert(hsd::IsArray<int[3]> && !hsd::IsArray<int*>);
static_assert(hsd::IsFunction<void()> && !hsd::IsFunction<void (*)()>);
static_assert(hsd::IsSame<int, int> && !hsd::IsSame<int, unsigned>);
static_assert(hsd::IsPointer<int*> && !hsd::IsPointer<int>);
static_assert(hsd::IsClass<point> && !hsd::IsClass<int> && !hsd::IsClass<int*>);
static_assert(hsd::IsChar<char> && hsd::IsChar<hsd::wchar> && hsd::IsChar<hsd::char16>);
static_assert(!hsd::IsChar<int>);

static_assert(hsd::NumericType<float> && hsd::IntegralType<int> && hsd::FloatingPointType<float>);
static_assert(hsd::SignedType<long> && hsd::UnsignedType<unsigned> && hsd::NaturalType<unsigned>);
static_assert(hsd::ObjectType<point> && hsd::VoidType<void> && hsd::ReferenceType<int&>);
static_assert(hsd::RvalueReferenceType<int&&> && hsd::LvalueReferenceType<int&>);
static_assert(hsd::ConstType<const int> && hsd::ArrayType<int[2]> && hsd::FunctionType<void()>);
static_assert(hsd::PointerType<int*> && hsd::CharacterType<char>);
static_assert(hsd::ClassType<point> && !hsd::ClassType<int>);

static_assert(hsd::Invocable<void (*)(int), int>);
static_assert(!hsd::Invocable<void (*)(int), const char*>);
static_assert(hsd::InvocableRet<int, int (*)(int), int>);
static_assert(!hsd::InvocableRet<int, void (*)(int), int>);

static_assert(hsd::Constructible<point, int, int> && !hsd::Constructible<point, int, int, int>);
static_assert(hsd::DefaultConstructible<int> && hsd::DefaultConstructible<point>);
static_assert(hsd::CopyConstructible<int> && hsd::CopyConstructible<point>);
static_assert(!hsd::CopyConstructible<move_only> && hsd::MoveConstructible<move_only>);
static_assert(hsd::CopyAssignable<int> && !hsd::CopyAssignable<move_only>);
static_assert(hsd::MoveAssignable<move_only>);

static_assert(hsd::Convertible<int, double> && !hsd::Convertible<int, int*>);
static_assert(hsd::RefConstructible<int&, int&> && !hsd::RefConstructible<int, int>);
static_assert(hsd::LiteralConstructible<int, int> && !hsd::LiteralConstructible<int, int, int>);

static_assert(hsd::BasicIterable<int*> && hsd::ForwardIterable<int*>);
static_assert(hsd::ReverseIterable<int*> && hsd::BidirectionalIterable<int*>);
static_assert(hsd::ContiguousForwardIterable<int*>);
static_assert(!hsd::BasicIterable<int> && !hsd::ForwardIterable<int>);

static_assert(hsd::IsForwardContainer<std::vector<int>>);
static_assert(hsd::IsReverseContainer<std::vector<int>>);
static_assert(hsd::IsBidirectionalContainer<std::vector<int>>);
static_assert(hsd::IsContainer<std::vector<int>> && !hsd::IsContainer<int>);

int main()
{
    return 0;
}
