using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_dead_end_detector_h2")]
    public static int DeadEndDetectorH2(int problemHandle)
        => CreateHandle(() => new H2DeadEndDetector(RequireHandle<Problem>(problemHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_dead_end_detector_disjunction")]
    public static unsafe int DeadEndDetectorDisjunction(IntPtr handles, int count)
        => CreateHandle(() =>
        {
            if (count < 0) throw new ArgumentOutOfRangeException(nameof(count));
            if (count > 0 && handles == IntPtr.Zero) throw new ArgumentNullException(nameof(handles));
            var detectors = new IDeadEndDetector[count];
            for (int index = 0; index < count; index++)
                detectors[index] = RequireHandle<IDeadEndDetector>(((int*)handles)[index]);
            return new DisjunctiveDeadEndDetector(detectors);
        });

    [UnmanagedCallersOnly(EntryPoint = "mimir_dead_end_detector_evaluate")]
    public static int DeadEndDetectorEvaluate(int detectorHandle, int stateHandle, int goalHandle)
        => ReadValue<IDeadEndDetector, int>(detectorHandle, -1, detector =>
            detector.IsDeadEnd(RequireHandle<ExtendedState>(stateHandle), RequireHandle<GoalCondition>(goalHandle)) ? 1 : 0);
}
