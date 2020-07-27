#include "cue.h"
#include <fmt/core.h>

namespace disc {
namespace format {
std::string Cue::getFile() const { return file; }

Position Cue::getDiskSize() const {
    int frames = 0;
    for (auto t : tracks) {
        frames += t.frames;
    }
    return Position::fromLba(frames) + Position{0, 2, 0};
}

size_t Cue::getTrackCount() const { return tracks.size(); }

Position Cue::getTrackStart(int track) const {
    size_t total = 0;

// DOOM: fix the first two seconds of cd audio being skipped.
// Only the data track needs the 2 second offset added: audio tracks are addressed using actual
// physical disc minutes and seconds and that amount which includes the disc header already...
//
// This issue was most noticable when entering the main menu and also on the finale screens.
#if PSYDOOM_AVOCADO_MODS
    if ((track == 0) && tracks.at(0).type == disc::TrackType::DATA) {
        total += 75 * 2;
    }
#else
    if (tracks.at(0).type == disc::TrackType::DATA) {
        total += 75 * 2;
    }
#endif

// DOOM: fix the track start being incorrectly determined. The calculation here should include the size of the track before
// the one we want... The previous code was causing issues with the looping cd audio in the 'CLUB DOOM' secret level because
// the track boundaries (and track end) were being computed incorrectly, causing the looping audio track to end prematurely...
#if PSYDOOM_AVOCADO_MODS
    for (int i = 0; i < track; i++) {
#else
    for (int i = 0; i < track - 1; i++) {
#endif
        total += tracks.at(i).frames;
    }
    return Position::fromLba(total);
}

Position Cue::getTrackLength(int track) const { return Position::fromLba(tracks.at(track).frames); }

int Cue::getTrackByPosition(Position pos) const {
    for (size_t i = 0; i < getTrackCount(); i++) {
        auto start = getTrackStart(i);
        auto size = tracks[i].frames;

        if (pos >= start && pos < start + Position::fromLba(size)) {
            return i;
        }
    }
    return -1;
}

disc::Sector Cue::read(Position pos) {
    auto buffer = std::vector<uint8_t>(Track::SECTOR_SIZE);
    auto type = disc::TrackType::INVALID;

    auto trackNum = getTrackByPosition(pos);
    if (trackNum != -1) {
        auto track = tracks[trackNum];
        if (files.find(track.filename) == files.end()) {
            auto f = unique_ptr_file(fopen(track.filename.c_str(), "rb"));
            if (!f) {
                fmt::print("Unable to load file {}\n", track.filename);
                return std::make_pair(buffer, type);
            }

            files.emplace(track.filename, std::move(f));
        }

        type = track.type;
        auto file = files[track.filename].get();

        if (trackNum == 0 && type == disc::TrackType::DATA) {
            pos = pos - Position{0, 2, 0};
        }
        auto seek = pos - *track.index0;
        fseek(file, (long)(track.offset + seek.toLba() * Track::SECTOR_SIZE), SEEK_SET);
        fread(buffer.data(), Track::SECTOR_SIZE, 1, file);
    }

    return std::make_pair(buffer, type);
}

std::unique_ptr<Cue> Cue::fromBin(const char* file) {
    auto size = getFileSize(file);
    if (size == 0) {
        return {};
    }

    Track t;
    t.filename = file;
    t.number = 1;
    t.type = disc::TrackType::DATA;
    t.pregap = Position(0, 0, 0);
    t.index0 = Position(0, 0, 0);
    t.index1 = Position(0, 0, 0);
    t.offset = 0;
    t.frames = size / Track::SECTOR_SIZE;

    auto cue = std::make_unique<Cue>();
    cue->file = file;
    cue->tracks.push_back(t);

    cue->loadSubchannel(file);

    return cue;
}
}  // namespace format
}  // namespace disc
