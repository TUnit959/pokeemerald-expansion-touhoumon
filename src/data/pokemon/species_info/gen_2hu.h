#ifdef __INTELLISENSE__
const struct SpeciesInfo gSpeciesInfoGen2hu[] =
{
#endif

#if P_FAMILY_CSANAE
    [SPECIES_CSANAE] =
    {
        .baseHP        = 65,
        .baseAttack    = 35,
        .baseDefense   = 90,
        .baseSpeed     = 45,
        .baseSpAttack  = 55,
        .baseSpDefense = 90,
        .types = { TYPE_GRASS, TYPE_GRASS },
        .catchRate = 45,
        .expYield = 62,
        .evYield_Speed = 1,
        .genderRatio = PERCENT_FEMALE(12.5),
        .eggCycles = 20,
        .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW,
        .eggGroups = { EGG_GROUP_MONSTER, EGG_GROUP_DRAGON },
        .abilities = { ABILITY_OVERGROW, ABILITY_NONE, ABILITY_UNBURDEN },
        .bodyColor = BODY_COLOR_GREEN,
        .speciesName = _("CSanae"),
        .cryId = CRY_TREECKO,
        .natDexNum = NATIONAL_DEX_TREECKO,
        .categoryName = _("Wood Gecko"),
        .height = 5,
        .weight = 50,
        .description = COMPOUND_STRING(
            "It makes its nest in a giant tree in the\n"
            "forest. It ferociously guards against\n"
            "anything nearing its territory. It is said\n"
            "to be the protector of the forest's trees."),
        .pokemonScale = 541,
        .pokemonOffset = 19,
        .trainerScale = 256,
        .trainerOffset = 0,
        FRONT_PIC(Treecko, 48, 48),
        .frontPicYOffset = 8,
        .frontAnimFrames = sAnims_Treecko,
        .frontAnimId = ANIM_V_SQUISH_AND_BOUNCE,
        BACK_PIC(Treecko, 56, 48),
        .backPicYOffset = 8,
        .backAnimId = BACK_ANIM_CONCAVE_ARC_LARGE,
        PALETTES(Treecko),
        ICON(Treecko, 1),
        .footprint = gMonFootprint_Treecko,
        LEARNSETS(Treecko),
        .evolutions = EVOLUTION({EVO_LEVEL, 16, SPECIES_SANAE}),
    },

    [SPECIES_SANAE] =
    {
        .baseHP        = 90,
        .baseAttack    = 60,
        .baseDefense   = 115,
        .baseSpeed     = 70,
        .baseSpAttack  = 80,
        .baseSpDefense = 115,
        .types = { TYPE_GRASS, TYPE_GRASS },
        .catchRate = 45,
        .expYield = 142,
        .evYield_Speed = 2,
        .genderRatio = PERCENT_FEMALE(12.5),
        .eggCycles = 20,
        .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW,
        .eggGroups = { EGG_GROUP_MONSTER, EGG_GROUP_DRAGON },
        .abilities = { ABILITY_OVERGROW, ABILITY_NONE, ABILITY_UNBURDEN },
        .bodyColor = BODY_COLOR_GREEN,
        .speciesName = _("Sanae"),
        .cryId = CRY_GROVYLE,
        .natDexNum = NATIONAL_DEX_GROVYLE,
        .categoryName = _("Wood Gecko"),
        .height = 9,
        .weight = 216,
        .description = COMPOUND_STRING(
            "Leaves grow out of this Pokémon's body.\n"
            "They help obscure a Grovyle from the eyes\n"
            "of its enemies while it is in a thickly\n"
            "overgrown forest."),
        .pokemonScale = 360,
        .pokemonOffset = 5,
        .trainerScale = 256,
        .trainerOffset = 0,
        FRONT_PIC(Grovyle, 64, 56),
        .frontPicYOffset = 5,
        .frontAnimFrames = sAnims_Grovyle,
        .frontAnimId = ANIM_V_STRETCH,
        BACK_PIC(Grovyle, 64, 56),
        .backPicYOffset = 7,
        .backAnimId = BACK_ANIM_JOLT_RIGHT,
        PALETTES(Grovyle),
        ICON(Grovyle, 1),
        .footprint = gMonFootprint_Grovyle,
        LEARNSETS(Grovyle),
        .evolutions = EVOLUTION({EVO_LEVEL, 36, SPECIES_SCEPTILE}),
    }
#ifdef __INTELLISENSE__
};
#endif
