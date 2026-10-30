/**
 * @file AvatarLoader.h
 * @brief Télécharge l'avatar du compte, le met en cache sur disque et le rend en cercle.
 *
 * Header-only : rien à ajouter au projet (ni .cpp, ni Projucer / CMake).
 * À placer à côté de HeaderComponent.h.
 *
 * Formats lus : PNG, JPEG, GIF (JUCE ne décode pas le WebP).
 */
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace AvatarLoader
{
    constexpr int avatarPx       = 128;               // net sur écran HiDPI
    constexpr int maxAvatarBytes = 2 * 1024 * 1024;

    inline bool isUsableUrl (const juce::String& url)
    {
        return url.startsWithIgnoreCase ("http://") || url.startsWithIgnoreCase ("https://");
    }

    inline juce::File cacheFileFor (const juce::String& url)
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Harmonia")
                   .getChildFile ("avatars")
                   .getChildFile (juce::String::toHexString (url.hashCode64()) + ".img");
    }

    // Recadre au centre et découpe en cercle
    inline juce::Image makeCircular (const juce::Image& src)
    {
        juce::Image out (juce::Image::ARGB, avatarPx, avatarPx, true);
        juce::Graphics g (out);

        juce::Path clip;
        clip.addEllipse (0.0f, 0.0f, (float) avatarPx, (float) avatarPx);
        g.reduceClipRegion (clip, {});

        const float scale = (float) avatarPx / (float) juce::jmin (src.getWidth(), src.getHeight());
        const float w = (float) src.getWidth()  * scale;
        const float h = (float) src.getHeight() * scale;

        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (src, juce::Rectangle<float> (((float) avatarPx - w) * 0.5f,
                                                  ((float) avatarPx - h) * 0.5f, w, h));
        return out;
    }

    // Image invalide si les octets ne sont pas une image lisible
    inline juce::Image decode (const void* data, size_t size)
    {
        const auto img = juce::ImageFileFormat::loadFrom (data, size);
        return img.isValid() ? makeCircular (img) : juce::Image();
    }

    // Instantané : image du cache disque, ou image invalide s'il n'y en a pas
    inline juce::Image loadFromCache (const juce::String& url)
    {
        const auto file = cacheFileFor (url);
        juce::MemoryBlock data;

        if (file.existsAsFile() && file.loadFileAsData (data))
            return decode (data.getData(), data.getSize());

        return {};
    }

    // Télécharge en arrière-plan. `onDone` est appelé sur le message thread,
    // avec une image invalide en cas d'échec (réseau, 404, fichier non lisible).
    // Penser à protéger le callback avec un SafePointer côté appelant.
    inline void fetchAsync (const juce::String& url, std::function<void (juce::Image)> onDone)
    {
        juce::Thread::launch ([url, onDone = std::move (onDone)]
        {
            int status = 0;
            juce::MemoryBlock data;

            if (auto in = juce::URL (url).createInputStream (
                    juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                        .withConnectionTimeoutMs (5000)
                        .withNumRedirectsToFollow (3)
                        .withStatusCode (&status)))
            {
                if (status == 200)
                    in->readIntoMemoryBlock (data, maxAvatarBytes + 1);
            }

            juce::Image img;

            if (data.getSize() > 0 && data.getSize() <= (size_t) maxAvatarBytes)
            {
                img = decode (data.getData(), data.getSize());

                if (img.isValid())
                {
                    const auto file = cacheFileFor (url);
                    file.getParentDirectory().createDirectory();
                    file.replaceWithData (data.getData(), data.getSize());
                }
            }

            juce::MessageManager::callAsync ([img, onDone]
            {
                if (onDone)
                    onDone (img);
            });
        });
    }
}