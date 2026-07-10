/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license.
 */

#ifndef PLAYERBOTS_PARTYMEMBERWITHOUTBLESSINGVALUE_H
#define PLAYERBOTS_PARTYMEMBERWITHOUTBLESSINGVALUE_H

#include "PartyMemberWithoutAuraValue.h"

class PartyMemberWithoutBlessingValue : public PartyMemberWithoutAuraValue
{
public:
    PartyMemberWithoutBlessingValue(PlayerbotAI* botAI,
                                    std::string const name = "party member without blessing")
        : PartyMemberWithoutAuraValue(botAI, name) {}

protected:
    Unit* Calculate() override;
};

#endif
