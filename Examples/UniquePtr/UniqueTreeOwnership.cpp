// Title: Tree ownership
// Level: Advanced
// Shows: a binary search tree where each node owns its children, re-linking subtrees by moving owners.

#include <Serialize/Io.hpp>
#include <Wrappers/UniquePtr.hpp>

using namespace hsd::format_literals;

struct node
{
    // Counts nodes alive, to prove that destroying the root frees the whole tree.
    static inline hsd::i32 alive = 0;

    hsd::i32 key;
    hsd::unique_ptr<node> left;
    hsd::unique_ptr<node> right;

    explicit node(hsd::i32 value)
        : key{value}
    {
        ++alive;
    }

    ~node()
    {
        --alive;
    }
};

using owner = hsd::unique_ptr<node>;

// Walks down with a pointer to the owning slot, so inserting is one assignment.
// The slot is a `owner*`: the root, or some node's left or right owner.
static bool insert(owner& root, hsd::i32 key)
{
    owner* slot = &root;

    while (*slot != nullptr)
    {
        if (key == (*slot)->key)
        {
            return false;
        }

        slot = key < (*slot)->key ? &(*slot)->left : &(*slot)->right;
    }

    *slot = hsd::make_unique<node>(key);
    return true;
}

static void print_in_order(const owner& root)
{
    if (root == nullptr)
    {
        return;
    }

    print_in_order(root->left);
    hsd::print(" {}"_fmt, root->key);
    print_in_order(root->right);
}

static hsd::i32 height(const owner& root)
{
    if (root == nullptr)
    {
        return 0;
    }

    hsd::i32 left_height = height(root->left);
    hsd::i32 right_height = height(root->right);
    return 1 + (left_height > right_height ? left_height : right_height);
}

// Detaches the smallest node: its right subtree takes its place.
// The detached node is returned, still owning nothing on its right.
static owner take_smallest(owner& root)
{
    owner* slot = &root;

    while ((*slot)->left != nullptr)
    {
        slot = &(*slot)->left;
    }

    owner smallest = hsd::move(*slot);
    *slot = hsd::move(smallest->right);
    return smallest;
}

// Swaps every pair of children by moving owners around.
static void mirror(owner& root)
{
    if (root == nullptr)
    {
        return;
    }

    owner old_left = hsd::move(root->left);
    root->left = hsd::move(root->right);
    root->right = hsd::move(old_left);
    mirror(root->left);
    mirror(root->right);
}

int main()
{
    owner root;
    const hsd::i32 keys[] = {50, 30, 70, 20, 40, 60, 80, 30, 65};
    hsd::i32 duplicates = 0;

    for (hsd::i32 key : keys)
    {
        if (!insert(root, key))
        {
            ++duplicates;
        }
    }

    hsd::print("in order:"_fmt);
    print_in_order(root);
    hsd::println(""_fmt);
    hsd::println("nodes alive: {} height: {} duplicates: {}"_fmt, node::alive, height(root), duplicates);

    // Take the two smallest keys out of the tree. Each comes back as its own owner.
    {
        owner first = take_smallest(root);
        owner second = take_smallest(root);
        hsd::println("took {} and {}"_fmt, first->key, second->key);
        hsd::println("nodes alive: {} (still owned by the locals)"_fmt, node::alive);
    }

    hsd::println("nodes alive after the locals ended: {}"_fmt, node::alive);

    hsd::print("in order:"_fmt);
    print_in_order(root);
    hsd::println(""_fmt);

    mirror(root);
    hsd::print("mirrored:"_fmt);
    print_in_order(root);
    hsd::println(""_fmt);

    // One assignment frees the entire tree, recursively through the owners.
    root = nullptr;
    hsd::println("nodes alive after reset: {}"_fmt, node::alive);
    return 0;
}
