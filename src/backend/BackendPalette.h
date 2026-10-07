#pragma once

// Compatibilité : tout vit maintenant dans storage/PaletteMapper.h.
// Les appels existants (BackendPalette::toHex / fromHex / readFromJson) continuent de compiler.
// À supprimer quand l'UI utilise PaletteMapper::.
#include "storage/PaletteMapper.h"

namespace BackendPalette = PaletteMapper;
