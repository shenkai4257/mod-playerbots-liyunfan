/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinTriggers.h"

#include "GenericBuffUtils.h"
#include "AiFactory.h"
// [[DEPRECATED]] — removed
//#include "PaladinBlessingActions.h"
#include "PaladinActions.h"
#include "PaladinHelper.h"
#include "Playerbots.h"

bool SealTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("seal of justice", target) && !botAI->HasAura("seal of command", target) &&
           !botAI->HasAura("seal of vengeance", target) && !botAI->HasAura("seal of corruption", target) &&
           !botAI->HasAura("seal of righteousness", target) && !botAI->HasAura("seal of light", target) &&
           (!botAI->HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 70);
}

bool CrusaderAuraTrigger::IsActive()
{
    Unit* target = GetTarget();
    return AI_VALUE2(bool, "mounted", "self target") && !botAI->HasAura("crusader aura", target);
}

// [[DEPRECATED]]
#if 0
bool BlessingTrigger::IsActive()
{
    // Iterate all reachable party members, check if any is missing their required blessing
    std::vector<Player*> targets;
    Group* group = bot->GetGroup();

    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* player = ref->GetSource();
            if (!player || !player->IsInWorld() || !player->IsAlive())
                continue;
            if (bot->GetMapId() != player->GetMapId())
                continue;
            if (bot->GetDistance(player) > sPlayerbotAIConfig.spellDistance * 2)
                continue;
            targets.push_back(player);
        }
    }
    else
    {
        targets.push_back(bot);
    }

    for (Player* target : targets)
    {
        // If target already has ANY blessing from us, skip entirely
        if (HasAnyBuffOfMe(target))
            continue;

        // Check blessings in per-class priority order
        auto priority = GetBlessingPriorityFor(target);
        for (std::string const& blessing : priority)
        {
            bool hasIt = botAI->HasAura(blessing, target) ||
                         botAI->HasAura("greater " + blessing, target);
            if (!hasIt)
            {
                std::string display = blessing.substr(11);
                std::ostringstream msg;
                msg << "缺少 " << display << " -> " << target->GetName();
                botAI->TellMaster(msg.str());
                return true;
            }
        }
    }

    return false;
}


bool BlessingTrigger::HasAnyBuffOfMe(Unit* target)
{
    return botAI->HasAura("blessing of might", target, false, true) ||
           botAI->HasAura("greater blessing of might", target, false, true) ||
           botAI->HasAura("blessing of wisdom", target, false, true) ||
           botAI->HasAura("greater blessing of wisdom", target, false, true) ||
           botAI->HasAura("blessing of kings", target, false, true) ||
           botAI->HasAura("greater blessing of kings", target, false, true) ||
           botAI->HasAura("blessing of sanctuary", target, false, true) ||
           botAI->HasAura("greater blessing of sanctuary", target, false, true);
}

#endif

bool DivineShieldLowHealthTrigger::IsActive()
{
    return botAI->HasAura("divine shield", bot) && AI_VALUE2(uint8, "health", "self target") < 80;
}

Unit* HandOfFreedomOnPartyTrigger::GetTarget()
{
    bool const selfImpaired = botAI->IsMovementImpaired(bot);
    bool const hasSelfHand = selfImpaired && ai::paladin::HasAnyPaladinHandFromCaster(bot, bot);

    if (!bot->GetGroup())
    {
        if (selfImpaired && !hasSelfHand)
            return bot;

        return nullptr;
    }

    if (selfImpaired && !hasSelfHand)
        return bot;

    return Trigger::GetTarget();
}

bool HandOfFreedomOnPartyTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (target != bot &&
        bot->GetExactDist2dSq(target->GetPositionX(), target->GetPositionY()) > 30.0f * 30.0f)
        return false;

    if (!botAI->CanCastSpell("hand of freedom", target))
        return false;

    return !ai::paladin::HasAnyPaladinHandFromCaster(target, bot) && botAI->IsMovementImpaired(target);
}

bool NotSensingUndeadTrigger::IsActive()
{
    return !botAI->HasAura("sense undead", bot);
}

// [[DEPRECATED]]
#if 0
bool GreaterBlessingNeededTrigger::IsActive()
{
    if (!ai::gbless::IsEligibleGroupForAutoBlessings(bot->GetGroup()))
        return false;

    if (ai::buff::ShouldDeferGreaterBlessingAssignmentForRecentLogin(bot))
        return false;

    Group* group = bot->GetGroup();
    uint32 const groupKey = group ? group->GetLeaderGUID().GetCounter() : 0;

    Value<ai::gbless::CachedPendingBlessingAssignment>* pendingValue =
        context->GetValue<ai::gbless::CachedPendingBlessingAssignment>("greater blessing pending assignment");
    if (!pendingValue)
        return false;

    ai::gbless::CachedPendingBlessingAssignment pendingAssignment = pendingValue->Get();
    if (pendingAssignment.groupKey != groupKey)
    {
        pendingValue->Reset();
        pendingAssignment = pendingValue->Get();
    }

    return pendingAssignment.valid && pendingAssignment.groupKey == groupKey;
}

#endif

