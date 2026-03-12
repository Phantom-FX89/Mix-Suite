#pragma once

#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace phantom::core {

template <typename Sample>
class AudioBlockView {
public:
    using Element = std::remove_const_t<Sample>;
    using Pointer = std::conditional_t<std::is_const_v<Sample>, const Element*, Element*>;

    AudioBlockView() = default;

    AudioBlockView(std::vector<Pointer> channels, std::size_t numSamples)
        : channels_(std::move(channels)), numSamples_(numSamples) {}

    [[nodiscard]] std::size_t numChannels() const noexcept { return channels_.size(); }
    [[nodiscard]] std::size_t numSamples() const noexcept { return numSamples_; }
    [[nodiscard]] bool empty() const noexcept { return channels_.empty() || numSamples_ == 0; }

    [[nodiscard]] std::span<Sample> channel(std::size_t channelIndex) const {
        if (channelIndex >= channels_.size()) {
            throw std::out_of_range("AudioBlockView channel out of range");
        }
        return {channels_[channelIndex], numSamples_};
    }

    [[nodiscard]] Sample& sample(std::size_t channelIndex, std::size_t sampleIndex) const {
        return channel(channelIndex).at(sampleIndex);
    }

private:
    std::vector<Pointer> channels_;
    std::size_t numSamples_ {0};
};

template <typename Sample>
class OwnedAudioBlock {
public:
    OwnedAudioBlock() = default;

    OwnedAudioBlock(std::size_t numChannels, std::size_t numSamples) {
        resize(numChannels, numSamples);
    }

    void resize(std::size_t numChannels, std::size_t numSamples) {
        numChannels_ = numChannels;
        numSamples_ = numSamples;
        storage_.assign(numChannels_ * numSamples_, Sample {});
        rebuildPointers();
    }

    void clear() {
        std::fill(storage_.begin(), storage_.end(), Sample {});
    }

    [[nodiscard]] std::size_t numChannels() const noexcept { return numChannels_; }
    [[nodiscard]] std::size_t numSamples() const noexcept { return numSamples_; }

    [[nodiscard]] AudioBlockView<Sample> view() {
        return {channelPointers_, numSamples_};
    }

    [[nodiscard]] AudioBlockView<const Sample> view() const {
        std::vector<const Sample*> constPointers;
        constPointers.reserve(channelPointers_.size());
        for (auto* pointer : channelPointers_) {
            constPointers.push_back(pointer);
        }
        return {std::move(constPointers), numSamples_};
    }

    [[nodiscard]] std::span<Sample> channel(std::size_t channelIndex) {
        return view().channel(channelIndex);
    }

    [[nodiscard]] std::span<const Sample> channel(std::size_t channelIndex) const {
        return view().channel(channelIndex);
    }

private:
    void rebuildPointers() {
        channelPointers_.clear();
        channelPointers_.reserve(numChannels_);
        for (std::size_t channel = 0; channel < numChannels_; ++channel) {
            channelPointers_.push_back(storage_.data() + (channel * numSamples_));
        }
    }

    std::size_t numChannels_ {0};
    std::size_t numSamples_ {0};
    std::vector<Sample> storage_;
    std::vector<Sample*> channelPointers_;
};

}  // namespace phantom::core
