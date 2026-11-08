#pragma once

#include <nxf/core/Error.h>
#include <nxf/core/Result.h>
#include <nxf/project/ProjectDocument.h>

#include <string>

namespace nxf::project {

/// Project persistence in .nxfproj files:
///   line 1:  NXF-PROJECT 1
///   rest:    the document root in NXF structured text
///
/// Saving is atomic: the content is written to "<path>.tmp" and
/// then renamed over the target, so a crash mid-save can never
/// leave a half-written project behind. Loading refuses a file
/// whose formatVersion is newer than this build understands
/// (Unavailable) instead of guessing at it.
[[nodiscard]] nxf::core::Error saveProject(ProjectDocument& document, const std::string& path);
[[nodiscard]] nxf::core::Result<ProjectDocument> loadProject(const std::string& path);

} // namespace nxf::project
