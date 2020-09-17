#include "PsxPatchVoice.h"

#include "InputStream.h"
#include "JsonUtils.h"
#include "Module.h"
#include "OutputStream.h"
#include "WmdFileTypes.h"

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch voice from json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    sampleIdx = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "sampleIdx", 0);
    volume = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "volume", 0);
    pan = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "pan", 0);
    reverb = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "reverb", 0);
    baseNote = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "baseNote", 0);
    baseNoteFrac = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "baseNoteFrac", 0);
    noteMin = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "noteMin", 0);
    noteMax = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "noteMax", 0);
    pitchstepDown = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "pitchstepDown", 0);
    pitchstepUp = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "pitchstepUp", 0);
    priority = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "priority", 0);
    adsr.sustainLevel = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_sustainLevel", 0), 0, 15);
    adsr.decayShift = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_decayShift", 0), 0, 15);
    adsr.attackStep = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_attackStep", 0), 0, 3);
    adsr.attackShift = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_attackShift", 0), 0, 31);
    adsr.bAttackExp = JsonUtils::getOrDefault<bool>(jsonRoot, "adsr_attackExponential", false);
    adsr.releaseShift = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_releaseShift", 0), 0, 31);
    adsr.bReleaseExp = JsonUtils::getOrDefault<bool>(jsonRoot, "adsr_releaseExponential", false);
    adsr.sustainStep = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_sustainStep", 0), 0, 3);
    adsr.sustainShift = (uint32_t) std::clamp<int64_t>(JsonUtils::getOrDefault<int64_t>(jsonRoot, "adsr_sustainShift", 0), 0, 31);
    adsr.bSustainDec = JsonUtils::getOrDefault<bool>(jsonRoot, "adsr_sustainDecrease", false);
    adsr.bSustainExp = JsonUtils::getOrDefault<bool>(jsonRoot, "adsr_sustainExponential", false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch voice to json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("sampleIdx", sampleIdx, jsonAlloc);
    jsonRoot.AddMember("volume", volume, jsonAlloc);
    jsonRoot.AddMember("pan", pan, jsonAlloc);
    jsonRoot.AddMember("reverb", reverb, jsonAlloc);
    jsonRoot.AddMember("baseNote", baseNote, jsonAlloc);
    jsonRoot.AddMember("baseNoteFrac", baseNoteFrac, jsonAlloc);
    jsonRoot.AddMember("noteMin", noteMin, jsonAlloc);
    jsonRoot.AddMember("noteMax", noteMax, jsonAlloc);
    jsonRoot.AddMember("pitchstepDown", pitchstepDown, jsonAlloc);
    jsonRoot.AddMember("pitchstepUp", pitchstepUp, jsonAlloc);
    jsonRoot.AddMember("priority", priority, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainLevel", adsr.sustainLevel, jsonAlloc);
    jsonRoot.AddMember("adsr_decayShift", adsr.decayShift, jsonAlloc);
    jsonRoot.AddMember("adsr_attackStep", adsr.attackStep, jsonAlloc);
    jsonRoot.AddMember("adsr_attackShift", adsr.attackShift, jsonAlloc);
    jsonRoot.AddMember("adsr_attackExponential", (bool) adsr.bAttackExp, jsonAlloc);
    jsonRoot.AddMember("adsr_releaseShift", adsr.releaseShift, jsonAlloc);
    jsonRoot.AddMember("adsr_releaseExponential", (bool) adsr.bReleaseExp, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainStep", adsr.sustainStep, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainShift", adsr.sustainShift, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainDecrease", (bool) adsr.bSustainDec, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainExponential", (bool) adsr.bSustainExp, jsonAlloc);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch voice from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::readFromWmdFile(InputStream& in) THROWS {
    WmdPsxPatchVoice wmdVoice = {};
    in.read(wmdVoice);
    wmdVoice.endianCorrect();

    this->sampleIdx = wmdVoice.sampleIdx;
    this->volume = wmdVoice.volume;
    this->pan = wmdVoice.pan;
    this->reverb = wmdVoice.reverb;
    this->baseNote = wmdVoice.baseNote;
    this->baseNoteFrac = wmdVoice.baseNoteFrac;
    this->noteMin = wmdVoice.noteMin;
    this->noteMax = wmdVoice.noteMax;
    this->pitchstepDown = wmdVoice.pitchstepDown;
    this->pitchstepUp = wmdVoice.pitchstepUp;
    this->priority = wmdVoice.priority;
    this->adsrBits = (uint32_t) wmdVoice.adsr1 | ((uint32_t) wmdVoice.adsr2 << 16);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch voice to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::writeToWmdFile(OutputStream& out) const THROWS {
    WmdPsxPatchVoice psxVoice = {};
    psxVoice.priority = priority;
    psxVoice.reverb = reverb;
    psxVoice.volume = volume;
    psxVoice.pan = pan;
    psxVoice.baseNote = baseNote;
    psxVoice.baseNoteFrac = baseNoteFrac;
    psxVoice.noteMin = noteMin;
    psxVoice.noteMax = noteMax;
    psxVoice.pitchstepDown = pitchstepDown;
    psxVoice.pitchstepUp = pitchstepUp;
    psxVoice.sampleIdx = sampleIdx;
    psxVoice.adsr1 = (uint16_t)(adsrBits);
    psxVoice.adsr2 = (uint16_t)(adsrBits >> 16);

    psxVoice.endianCorrect();
    out.write(psxVoice);
}

END_NAMESPACE(AudioTools)
