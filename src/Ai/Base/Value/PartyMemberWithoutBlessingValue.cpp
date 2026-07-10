/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license.
 */

#include "PartyMemberWithoutBlessingValue.h"

#include "AiFactory.h"
#include "Playerbots.h"

extern std::vector<std::string> split(std::string const s, char delim);

namespace
{
    bool HasAnyBlessingFromMe(PlayerbotAI* botAI, Unit* target)
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

    // ── Class‑spec helpers ──

    bool IsPhysicalDpsClass(Player* player)
    {
        if (!player) return false;
        switch (player->getClass())
        {
            case CLASS_WARRIOR: case CLASS_ROGUE:
            case CLASS_HUNTER: case CLASS_DEATH_KNIGHT:
                return true;
            case CLASS_SHAMAN:
                return AiFactory::GetPlayerSpecTab(player) == SHAMAN_TAB_ENHANCEMENT;
            case CLASS_DRUID:
                return AiFactory::GetPlayerSpecTab(player) == DRUID_TAB_FERAL;
            case CLASS_PALADIN:
                return AiFactory::GetPlayerSpecTab(player) == PALADIN_TAB_RETRIBUTION;
            default: return false;
        }
    }

    bool IsCasterDpsClass(Player* player)
    {
        if (!player) return false;
        switch (player->getClass())
        {
            case CLASS_MAGE: case CLASS_WARLOCK: case CLASS_PRIEST:
                return true;
            case CLASS_SHAMAN:
                return AiFactory::GetPlayerSpecTab(player) == SHAMAN_TAB_ELEMENTAL;
            case CLASS_DRUID:
                return AiFactory::GetPlayerSpecTab(player) == DRUID_TAB_BALANCE;
            default: return false;
        }
    }

    // ── Role filters ──

    std::vector<Player*> FilterByPhysical(PlayerbotAI* botAI, std::vector<Player*> const& players)
    {
        std::vector<Player*> result; result.reserve(players.size());
        for (Player* p : players) {
            if (!p) continue;
            if (botAI->IsTank(p) || botAI->IsHeal(p)) continue;
            if (IsPhysicalDpsClass(p)) result.push_back(p);
        }
        return result;
    }

    std::vector<Player*> FilterByCaster(PlayerbotAI* botAI, std::vector<Player*> const& players)
    {
        std::vector<Player*> result; result.reserve(players.size());
        for (Player* p : players) {
            if (!p) continue;
            if (botAI->IsTank(p)) continue;
            if (botAI->IsHeal(p) || IsCasterDpsClass(p)) result.push_back(p);
        }
        return result;
    }

    std::vector<Player*> FilterByTank(PlayerbotAI* botAI, std::vector<Player*> const& players)
    {
        std::vector<Player*> result; result.reserve(players.size());
        for (Player* p : players) {
            if (!p) continue;
            if (botAI->IsTank(p)) result.push_back(p);
        }
        return result;
    }

    // ── Party ordering ──

    std::vector<Player*> GetPartyByRoleOrder(Player* bot, PlayerbotAI* botAI)
    {
        std::vector<Player*> masters, healers, tanks, dps;
        Player* master = botAI->GetMaster();
        Group* group = bot->GetGroup();
        if (!group) { if (bot->IsAlive()) return {bot}; return {}; }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* player = ref->GetSource();
            if (!player || !player->IsAlive()) continue;
            if (bot->GetMapId() != player->GetMapId()) continue;
            if (bot->GetDistance(player) > sPlayerbotAIConfig.spellDistance * 2) continue;
            if (player == master) masters.push_back(player);
            else if (botAI->IsHeal(player)) healers.push_back(player);
            else if (botAI->IsTank(player)) tanks.push_back(player);
            else dps.push_back(player);
        }

        std::vector<Player*> result;
        for (Player* p : masters) result.push_back(p);
        for (Player* p : healers) result.push_back(p);
        for (Player* p : tanks) result.push_back(p);
        for (Player* p : dps) result.push_back(p);
        return result;
    }

    // ── Blessing‑type routing ──

    bool IsMightBlessing(std::string const& q)     { return q.find("might") != std::string::npos; }
    bool IsWisdomBlessing(std::string const& q)    { return q.find("wisdom") != std::string::npos; }
    bool IsSanctuaryBlessing(std::string const& q) { return q.find("sanctuary") != std::string::npos; }

    // ── Per‑player predicate ──

    class PlayerWithoutBlessingPredicate : public FindPlayerPredicate, public PlayerbotAIAware
    {
    public:
        PlayerWithoutBlessingPredicate(PlayerbotAI* botAI, std::string const& aura)
            : FindPlayerPredicate(), PlayerbotAIAware(botAI), auras(split(aura, ',')) {}

        bool Check(Unit* unit) override
        {
            if (!unit->IsAlive()) return false;
            for (auto const& a : auras)
                if (botAI->HasAura(a, unit)) return false;
            if (HasAnyBlessingFromMe(botAI, unit)) return false;
            return true;
        }
    private:
        std::vector<std::string> auras;
    };
}

// ── Main entry point ──

Unit* PartyMemberWithoutBlessingValue::Calculate()
{
    PlayerWithoutBlessingPredicate pred(botAI, qualifier);
    auto all = GetPartyByRoleOrder(bot, botAI);

    if (IsMightBlessing(qualifier))
    {
        for (Player* p : FilterByPhysical(botAI, all))
            if (pred.Check(p)) return p;
        for (Player* p : all) if (pred.Check(p)) return p;
        return nullptr;
    }
    if (IsWisdomBlessing(qualifier))
    {
        for (Player* p : FilterByCaster(botAI, all))
            if (pred.Check(p)) return p;
        for (Player* p : all) if (pred.Check(p)) return p;
        return nullptr;
    }
    if (IsSanctuaryBlessing(qualifier))
    {
        for (Player* p : FilterByTank(botAI, all))
            if (pred.Check(p)) return p;
        for (Player* p : all) if (pred.Check(p)) return p;
        return nullptr;
    }
    for (Player* p : all) if (pred.Check(p)) return p;
    return nullptr;
}
