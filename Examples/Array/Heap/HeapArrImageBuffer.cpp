// Title: Heap array image buffer
// Level: Advanced
// Shows: a heap_array as a pixel buffer with a custom element type, shapes, and copy-on-snapshot.

#include <Containers/HeapArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

constexpr hsd::usize width = 16;
constexpr hsd::usize height = 8;

// A small value type: heap_array only needs it to be default constructible
// and copyable.
struct pixel
{
    char shade = '.';
};

using image = hsd::heap_array<pixel, width * height>;

// Writes one pixel, ignoring coordinates outside the image. at() reports the
// bad index as an error, so no unchecked write can happen.
void put(image& img, hsd::isize x, hsd::isize y, char shade)
{
    if (x < 0 || y < 0 || x >= static_cast<hsd::isize>(width)
        || y >= static_cast<hsd::isize>(height))
    {
        return;
    }

    auto slot = img.at(static_cast<hsd::usize>(y) * width + static_cast<hsd::usize>(x));
    if (slot.is_ok())
    {
        slot.unwrap().get().shade = shade;
    }
}

void fill_rect(image& img, hsd::isize left, hsd::isize top,
    hsd::isize rect_width, hsd::isize rect_height, char shade)
{
    for (hsd::isize y = top; y < top + rect_height; ++y)
    {
        for (hsd::isize x = left; x < left + rect_width; ++x)
        {
            put(img, x, y, shade);
        }
    }
}

// Midpoint-free integer circle outline: test every pixel against the radius.
void draw_circle(image& img, hsd::isize cx, hsd::isize cy, hsd::isize radius, char shade)
{
    for (hsd::isize y = cy - radius; y <= cy + radius; ++y)
    {
        for (hsd::isize x = cx - radius; x <= cx + radius; ++x)
        {
            hsd::isize dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            hsd::isize inner = (radius - 1) * (radius - 1);
            if (dist <= radius * radius && dist > inner)
            {
                put(img, x, y, shade);
            }
        }
    }
}

void show(const char* label, const image& img)
{
    hsd::println("{}:"_fmt, label);
    for (hsd::usize y = 0; y < height; ++y)
    {
        for (hsd::usize x = 0; x < width; ++x)
        {
            hsd::print("{}"_fmt, img[y * width + x].shade);
        }
        hsd::println(""_fmt);
    }
}

hsd::usize count_shade(const image& img, char shade)
{
    hsd::usize total = 0;
    for (const auto& px : img)
    {
        if (px.shade == shade)
        {
            ++total;
        }
    }
    return total;
}

int main()
{
    image canvas;
    show("blank", canvas);

    fill_rect(canvas, 1, 1, 5, 3, '#');
    // This rectangle sticks out of the image and gets clipped.
    fill_rect(canvas, 12, 5, 8, 6, '=');

    // Copying the buffer takes a snapshot; later edits do not touch it.
    image snapshot{canvas};

    draw_circle(canvas, 8, 3, 3, 'o');
    show("drawn", canvas);
    show("snapshot", snapshot);

    hsd::println("counts: # {} o {} = {}"_fmt,
        count_shade(canvas, '#'), count_shade(canvas, 'o'), count_shade(canvas, '='));
    hsd::println("snapshot o pixels: {}"_fmt, count_shade(snapshot, 'o'));

    // Moving hands the whole buffer over without copying 128 pixels.
    image final_image{hsd::move(canvas)};
    hsd::println("final o pixels: {}"_fmt, count_shade(final_image, 'o'));

    return 0;
}
