# Contraste des couleurs : la règle que suit Harmonia

Le plugin vise le niveau **AA des WCAG 2.1** (critère 1.4.3, « Contraste minimum ») pour tout texte affiché avec **les 3 couleurs par défaut de la palette**. Si l'utilisateur choisit une autre couleur, le contraste dépend de son choix : on ne le garantit pas.

La règle est vérifiée automatiquement par [`tests/ContrastTests.cpp`](tests/ContrastTests.cpp), avec la formule de [`src/themes/Contrast.h`](src/themes/Contrast.h).

---

## 1. La formule du contraste (WCAG 2.1)

### Luminance relative d'une couleur

Chaque canal sRGB (0 à 255) est ramené entre 0 et 1, puis linéarisé :

```
s = canal / 255
c = s / 12.92                        si s <= 0.03928
c = ((s + 0.055) / 1.055) ^ 2.4      sinon
```

La luminance relative pondère les trois canaux selon la sensibilité de l'œil (le vert compte le plus) :

```
L = 0.2126 · R + 0.7152 · G + 0.0722 · B
```

`L` va de 0 (noir) à 1 (blanc).

### Ratio de contraste

```
ratio = (L_clair + 0.05) / (L_sombre + 0.05)
```

Le ratio va de **1:1** (même couleur) à **21:1** (noir sur blanc).

### Couleurs transparentes

Une couleur avec de l'alpha est d'abord **posée sur son fond**, comme à l'écran :

```
affichée = alpha · texte + (1 − alpha) · fond      (canal par canal)
```

On calcule ensuite le ratio entre cette couleur affichée et le fond. C'est pour ça qu'un `withAlpha (0.5f)` fait souvent passer un texte sous la norme.

### Seuils

| Cas | Ratio minimum |
|---|---|
| Texte normal (moins de 24 px, ou moins de 18,7 px en gras) | **4,5:1** |
| Gros texte (24 px et plus, ou 18,7 px et plus en gras) | 3:1 |
| Éléments non textuels utiles (icônes, contours de champs), critère 1.4.11 | 3:1 |

Presque tout le texte du plugin fait entre 8 et 14 px : **c'est le seuil de 4,5:1 qui s'applique.**

---

## 2. Comment le plugin construit ses couleurs de texte

En mode palette personnalisée (le mode par défaut), tout dérive de la **couleur d'accent** `c` choisie dans les 3 pastilles. Voir `HarmoniaPalette::setCustomAccent` dans [`src/themes/HarmoniaPalette.h`](src/themes/HarmoniaPalette.h).

| Couleur | Formule | Rôle |
|---|---|---|
| `accent` | `c` | valeurs des knobs, titres Osc / Display, placeholder Refine |
| `textPrimary` | `c` mélangée à **88 %** de blanc | titres, texte principal |
| `textSecondary` | `c` mélangée à **55 %** de blanc | labels, dates |
| `textMuted` | `c` en HSB : saturation × **0,5**, luminosité **0,75** | libellés des knobs, nom du preset, « AI MODEL », hex… |

« Mélangée à x % de blanc » veut dire `couleur = c + (blanc − c) · x`, canal par canal (`juce::Colour::interpolatedWith`).

Ces coefficients sont des constantes nommées (`textPrimaryMix`, `textSecondaryMix`, `textMutedSat`, `textMutedBright`). **Si tu les modifies, relance les tests de contraste.**

### Les fonds (fixes, ils ne changent pas avec la palette)

| Fond | Couleur | Où |
|---|---|---|
| `background` | `#1E2226` | fond général |
| `panel` / `panelTop` | `#262A30` / `#2A2F35` | panneaux de section, header |
| `panel.darker (0.55)` | `#181B1F` | capsule du prompt |
| `panel.darker (0.45)` + noir à 25 % | ≈ `#141619` | cartes d'historique |
| `screenBg` | `#0A0D11` | écran de l'oscilloscope |

`darker (a)` multiplie chaque canal par `1 / (1 + a)`, et `brighter (a)` rapproche du blanc dans la même proportion.

### La couleur d'accent elle-même

L'accent sert directement de couleur de texte (valeurs, titres de section). La règle n'impose rien à l'utilisateur, mais **une couleur de pastille par défaut doit avoir au moins 4,5:1 sur `panelTop` (#2A2F35)**. Les trois couleurs actuelles passent :

| Pastille | Couleur | Ratio sur `panelTop` |
|---|---|---|
| 1 | `#00D4FF` cyan | 7,62 |
| 2 | `#7B8CFF` violet | 4,52 (la limite basse) |
| 3 | `#9AA5B8` gris bleuté | 5,43 |

Pour changer une couleur par défaut (`HarmoniaPalette::defaultSlots`), choisis-en une d'au moins 4,5:1 sur `#2A2F35`.

---

## 3. Résultats avec les 3 couleurs par défaut

Les ratios ont été calculés avec les formules ci-dessus, sur les couleurs exactes du code.

| Élément | Taille | Cyan `00D4FF` | Violet `7B8CFF` | Gris `9AA5B8` |
|---|---|---|---|---|
| Libellés des knobs | 9,5 px | 5,76 | 4,91 | 7,03 |
| Valeur des knobs | 10 px | 8,14 | 4,83 | 5,80 |
| Titre de section Osc / Display | 10 px | 7,62 | 4,52 | 5,43 |
| Nom du preset | 10 px | 5,39 | 4,59 | 6,57 |
| Placeholder du prompt (Generate) | 14 px | 8,41 | 7,53 | 9,86 |
| Placeholder du prompt (Refine) | 14 px | 9,74 | 5,78 | 6,94 |
| « AI MODEL » | 8 px | 6,40 | 5,45 | 7,80 |
| Labels | 12 px | 11,83 | 10,17 | 11,02 |
| Hex de la palette | 11 px | 5,39 | 4,59 | 6,57 |
| Historique : titre | 12,5 px | 15,41 | 15,23 | 15,39 |
| Historique : modèle | 11 px | 12,65 | 12,50 | 12,64 |
| Historique : date | 10,5 px | 12,40 | 10,86 | 11,65 |
| Historique vide | 12 px | 6,76 | 5,75 | 8,24 |
| « OSCILLOSCOPE » | 9 px | 7,78 | 6,62 | 9,49 |

Couleurs de section fixes, identiques pour toutes les palettes : Filter `#A78BFA` 4,96 · LFO `#34D399` 7,02 · FX `#EB6F92` 4,64 · Amp `#FBBF24` 8,08.

### Pages de connexion (couleurs fixes, `HarmoniaColours`)

| Élément | Ratio |
|---|---|
| Sous-titre (`textSubtitle #94A3B8`) | 6,92 |
| Placeholders des champs (blanc à 50 %) | 4,5 et plus |
| Bouton « Back » (blanc à 50 %, 80 % au survol) | 4,6 et plus |
| `textMuted #7C8BA1` | 5,1 et plus |

---

## 4. Ce qui a été corrigé

| Avant | Ratio avant | Correction |
|---|---|---|
| `textMuted` à luminosité 0,5 | 2,29 à 3,86 | luminosité **0,75** |
| « OSCILLOSCOPE » en accent à 50 % | 2,45 à 3,43 | couleur `textMuted` |
| Placeholder Refine en accent à 80 % | 4,17 (violet) | accent opaque |
| Sous-titre de connexion `waveBlue` à 65 % | 2,82 | `textSubtitle` |
| Placeholders de connexion, blanc à 35 % | 3,15 | blanc à 50 % |
| Bouton « Back », blanc à 35 % | 3,21 | blanc à 50 % |
| `HarmoniaColours::textMuted #64748B` | 3,98 | `#7C8BA1` |

---

## 5. Les règles à respecter quand on ajoute du texte

1. **Pas de texte en `withAlpha` sous 1.0** sans recalculer le ratio. La transparence divise vite le contraste.
2. **Texte discret = `textMuted`**, jamais un accent assombri ou transparent fait à la main.
3. **Nouveau texte sur un nouveau fond** : ajoute la paire dans `tests/ContrastTests.cpp`. Le test échoue s'il descend sous 4,5:1 avec une des 3 couleurs par défaut.
4. **Vérifier un ratio à la main** :

```cpp
#include "themes/Contrast.h"
const double r = Contrast::ratio (textColour, backgroundColour);   // >= Contrast::minText (4.5)
```

Lancer uniquement ces tests :

```powershell
.\build\tests\Release\Harmonia_Tests.exe "[contrast]"
```

### Hors périmètre

Les thèmes prédéfinis (Cyan, Blue, Red, Purple, Dark, Green dans `HarmoniaPalette::Themes`) et les couleurs choisies par l'utilisateur ne sont pas couverts par cette garantie.
