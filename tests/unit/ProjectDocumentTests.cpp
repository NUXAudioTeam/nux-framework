#include <nxf/project/ProjectDocument.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::project;
    ProjectDocument doc("Demo");
    NXF_CHECK(doc.name() == "Demo");
    NXF_CHECK(!doc.isDirty());                                // a fresh document is clean
    NXF_CHECK(doc.value("formatVersion")->asInt() == 1);

    doc.checkpoint("set tempo");
    NXF_CHECK(doc.setValue("settings.tempo", Value::real(140.0)));
    NXF_CHECK(doc.isDirty());
    NXF_CHECK(doc.value("settings.tempo")->asReal() == 140.0);
    doc.markSaved();
    NXF_CHECK(!doc.isDirty());
    NXF_CHECK(doc.canUndo());
    NXF_CHECK(doc.undo());
    NXF_CHECK(doc.value("settings.tempo") == nullptr);        // edit undone
    NXF_CHECK(doc.isDirty());                                 // ...so it differs from saved again
    NXF_CHECK(doc.canRedo());
    NXF_CHECK(doc.redo());
    NXF_CHECK(doc.value("settings.tempo")->asReal() == 140.0);
    NXF_CHECK(!doc.isDirty());

    // coalescing: a knob drag is ONE undo step, not fifty
    ProjectDocument drag("Drag");
    drag.checkpoint("drag gain", "gain:track1");
    drag.setValue("gain", Value::real(0.1));
    drag.checkpoint("drag gain", "gain:track1");              // same key: coalesced
    drag.setValue("gain", Value::real(0.9));
    NXF_CHECK(drag.undo());
    NXF_CHECK(drag.value("gain") == nullptr);                 // straight back to before the drag
    NXF_CHECK(!drag.canUndo());

    // a new edit after undo clears the redo branch
    ProjectDocument branch("Branch");
    branch.checkpoint("a"); branch.setValue("x", Value::integer(1));
    branch.undo();
    branch.checkpoint("b"); branch.setValue("x", Value::integer(2));
    NXF_CHECK(!branch.canRedo());
    NXF_CHECK(branch.value("x")->asInt() == 2);
    NXF_CHECK(!branch.setValue("", Value::integer(1)));       // invalid paths refused
    return nxf::test::report("ProjectDocumentTests");
}
