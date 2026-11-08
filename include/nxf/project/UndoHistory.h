#pragma once

#include <nxf/project/Value.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace nxf::project {

/// Snapshot-based undo over Value states, bounded in depth.
///
/// The caller records the state *before* an edit (checkpoint in
/// ProjectDocument does this). Edits made while a control is being
/// dragged should share a coalesce key: repeated records with the
/// same key keep only the first snapshot, so one drag is one undo
/// step - the exact behaviour NUX DAW learned the hard way. Call
/// breakCoalescing() when a new gesture begins.
class UndoHistory {
public:
    explicit UndoHistory(std::size_t maxDepth = 100) : maxDepth_(maxDepth == 0 ? 1 : maxDepth) {}

    void record(const Value& stateBeforeEdit, const std::string& label, const std::string& coalesceKey = "") {
        if (!coalesceKey.empty() && coalesceKey == lastCoalesceKey_ && !undoStack_.empty()) {
            redoStack_.clear();
            return;   // same gesture: the original snapshot already covers it
        }
        undoStack_.push_back(stateBeforeEdit);
        if (undoStack_.size() > maxDepth_) undoStack_.erase(undoStack_.begin());
        redoStack_.clear();
        lastLabel_ = label;
        lastCoalesceKey_ = coalesceKey;
    }

    [[nodiscard]] std::optional<Value> undo(const Value& currentState) {
        if (undoStack_.empty()) return std::nullopt;
        redoStack_.push_back(currentState);
        Value previous = undoStack_.back();
        undoStack_.pop_back();
        breakCoalescing();
        return previous;
    }

    [[nodiscard]] std::optional<Value> redo(const Value& currentState) {
        if (redoStack_.empty()) return std::nullopt;
        undoStack_.push_back(currentState);
        Value next = redoStack_.back();
        redoStack_.pop_back();
        breakCoalescing();
        return next;
    }

    [[nodiscard]] bool canUndo() const noexcept { return !undoStack_.empty(); }
    [[nodiscard]] bool canRedo() const noexcept { return !redoStack_.empty(); }
    [[nodiscard]] const std::string& lastLabel() const noexcept { return lastLabel_; }
    void breakCoalescing() noexcept { lastCoalesceKey_.clear(); }
    void clear() {
        undoStack_.clear();
        redoStack_.clear();
        breakCoalescing();
    }

private:
    std::vector<Value> undoStack_;
    std::vector<Value> redoStack_;
    std::string lastLabel_;
    std::string lastCoalesceKey_;
    std::size_t maxDepth_;
};

} // namespace nxf::project
