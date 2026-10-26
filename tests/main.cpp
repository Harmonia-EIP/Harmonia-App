#include <catch2/catch_session.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    return Catch::Session().run (argc, argv);
}