#pragma once

// Discord Rich Presence for Shadow The Hedgehog.
//
// This is a straight 1:1 C++ port of Sewer's original C# Reloaded-II mod for Dolphin.
// https://github.com/Sewer56/ShadowTheHedgehog.RPC.ReloadedII
//
// Differences from the original:
//   - The "offensive captions" setting is not ported.
//   - Fixed handling of FMVs.
//   - Minor presentation changes.
//   - Handle missions at end of story properly (e.g. no Neutral in Lava Shelter)

namespace RichPresence::ShadowRpc
{
    // Starts updating the presence in the background.
    void Start();

    // Stops updating the presence and clears it from Discord.
    void Stop();
}
