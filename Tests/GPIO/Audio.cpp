#include <Extra/GPIO.hpp>
#include <Thread.hpp>

void play_tone(hsd::gpio::pin_ref& pin, hsd::f32 tone, hsd::i32 duration)
{
    auto step = static_cast<hsd::i32>(tone * 1000000.f); 

    for (hsd::i32 i = 0; i < duration * 1000; i += step * 2)
    {
        pin.get().set_value(hsd::gpio::value::high).unwrap();
        hsd::this_thread::sleep_for(tone).unwrap();

        pin.get().set_value(hsd::gpio::value::low).unwrap();
        hsd::this_thread::sleep_for(tone).unwrap();
    }
}

void play_note(hsd::gpio::pin_ref& pin, char note, hsd::i32 duration)
{
    constexpr char names[] = {
        'C', 'D', 'E', 'F', 'G', 'A', 'B',
        'c', 'd', 'e', 'f', 'g', 'a', 'b',
        'x', 'y'};

    constexpr hsd::f32 tones[] = {
        0.001915f, 0.001700f, 0.001519f, 0.001432f, 0.001275f, 0.001136f, 0.001014f,
        0.000956f, 0.000834f, 0.000765f, 0.000593f, 0.000468f, 0.000346f, 0.000224f,
        0.000655f, 0.000715f};

    hsd::i32 spee = 5;

    // play the tone corresponding to the note name

    for (int i = 0; i < 16; i++)
    {
        if (names[i] == note)
        {
            hsd::i32 new_duration = duration / spee;
            play_tone(pin, tones[i], new_duration);
        }
    }
}

int main()
{
    auto audio_out = hsd::gpio::get_pin(hsd::gpio::pins::pin_4);
    audio_out.get().set_direction(hsd::gpio::direction::output);

    constexpr char notes[] = "GGAGcB GGAGdc GGxecBA yyecdc";

    constexpr hsd::f32 beats[] = {
        0.002, 0.002, 0.008, 0.008, 0.008, 0.016,
        0.001, 0.002, 0.002, 0.008, 0.008, 0.008,
        0.016, 0.001, 0.002, 0.002, 0.008, 0.008,
        0.008, 0.008, 0.016, 0.001, 0.002, 0.002,
        0.008, 0.008, 0.008, 0.016
    };

    constexpr hsd::f32 tempo = 150.f;
    constexpr hsd::i32 length = 28;

    for (auto times = 0; times < 2; times++)
    {
        for (hsd::i32 i = 0; i < length; i++)
        {
            if (notes[i] == ' ')
            {
                // Rest
                hsd::this_thread::sleep_for(beats[i] * tempo).unwrap();
            }
            else
            {
                play_note(
                    audio_out, notes[i], 
                    static_cast<hsd::i32>(beats[i] * tempo * 1000.f)
                );
            }

            // Pause between notes
            hsd::this_thread::sleep_for(tempo * 0.001).unwrap();
        }
    }
}