#pragma once

#include <nxf/project/UndoHistory.h>

#include <string>

namespace nxf::project {

/// An editable project document: a root Value object with a format
/// version and a name, dot-path access ("settings.tempo", array
/// indices as numbers), dirty tracking against the last saved state,
/// and checkpoint-based undo/redo.
///
/// Dirty means "the current state serialises differently from the
/// saved state" - an exact comparison, so undoing back to the saved
/// state makes the document clean again.
class ProjectDocument {
public:
    static constexpr std::int64_t currentFormatVersion = 1;

    explicit ProjectDocument(std::string name = "Untitled");
    explicit ProjectDocument(Value root);

    [[nodiscard]] const Value& root() const noexcept { return root_; }
    [[nodiscard]] std::string name() const;
    void setName(const std::string& name);

    /// Dot-path lookup; nullptr when any step is missing/mistyped.
    [[nodiscard]] const Value* value(const std::string& path) const;
    /// Creates intermediate objects (or arrays before numeric steps)
    /// as needed. False for an invalid path or a scalar in the way.
    bool setValue(const std::string& path, Value newValue);

    /// Records the current state as an undo checkpoint. Call before
    /// mutating; share a coalesce key across one continuous gesture.
    void checkpoint(const std::string& label, const std::string& coalesceKey = "") {
        history_.record(root_, label, coalesceKey);
    }
    [[nodiscard]] bool undo() {
        if (auto previous = history_.undo(root_)) { root_ = std::move(*previous); return true; }
        return false;
    }
    [[nodiscard]] bool redo() {
        if (auto next = history_.redo(root_)) { root_ = std::move(*next); return true; }
        return false;
    }
    [[nodiscard]] bool canUndo() const noexcept { return history_.canUndo(); }
    [[nodiscard]] bool canRedo() const noexcept { return history_.canRedo(); }

    [[nodiscard]] bool isDirty() const { return serializeValue(root_) != savedCanonical_; }
    void markSaved() { savedCanonical_ = serializeValue(root_); }

private:
    Value root_;
    UndoHistory history_;
    std::string savedCanonical_;
};

} // namespace nxf::project
