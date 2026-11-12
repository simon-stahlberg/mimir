using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed class SerialEditResult
{
    public int AppliedCount { get; }
    public IReadOnlyList<int> DroppedErrorIndices { get; }
    public IReadOnlyList<int> CreatedStepIds { get; }
    public IReadOnlyList<int> DeletedStepIds { get; }

    internal SerialEditResult(int appliedCount, IReadOnlyList<int> droppedErrorIndices, IReadOnlyList<int> createdStepIds, IReadOnlyList<int> deletedStepIds)
    {
        AppliedCount = appliedCount;
        DroppedErrorIndices = droppedErrorIndices;
        CreatedStepIds = createdStepIds;
        DeletedStepIds = deletedStepIds;
    }
}

public sealed partial class SerialPlan
{
    public SerialEditResult Apply(SerialDiagnosis diagnosis, IReadOnlyList<int> chosenRepairIndex, IReadOnlyList<int> stepIdsToDelete)
    {
        ArgumentNullException.ThrowIfNull(diagnosis);
        ArgumentNullException.ThrowIfNull(chosenRepairIndex);
        ArgumentNullException.ThrowIfNull(stepIdsToDelete);
        if (!ReferenceEquals(diagnosis.Plan, this) || diagnosis.Version != _version)
            throw new InvalidOperationException("The diagnosis was not computed from this plan in its current state.");
        if (chosenRepairIndex.Count != diagnosis.Errors.Count)
            throw new ArgumentException($"Expected {diagnosis.Errors.Count} repair choices, got {chosenRepairIndex.Count}.", nameof(chosenRepairIndex));

        var chosen = new SerialRepair[diagnosis.Errors.Count];
        for (int i = 0; i < chosen.Length; i++)
        {
            IReadOnlyList<SerialRepair> repairs = diagnosis.Errors[i].Repairs;
            if (chosenRepairIndex[i] < 0 || chosenRepairIndex[i] >= repairs.Count)
                throw new ArgumentOutOfRangeException(nameof(chosenRepairIndex), chosenRepairIndex[i], $"Error {i} has {repairs.Count} repairs.");
            chosen[i] = repairs[chosenRepairIndex[i]];
        }

        var deleted = new List<int>();
        foreach (int id in stepIdsToDelete)
        {
            if (IndexOfStep(id) < 0) throw new ArgumentException($"Step {id} is not part of the plan.", nameof(stepIdsToDelete));
            if (!deleted.Contains(id)) deleted.Add(id);
        }

        _steps.RemoveAll(step => deleted.Contains(step.Id));
        int applied = deleted.Count;
        var dropped = new List<int>();

        // A repair was offered against the binding at diagnosis time, so one that follows another change
        // to the same step is checked again, and a parameter is changed at most once per round.
        var changedParameters = new HashSet<(int StepId, int ParameterIndex)>();
        for (int i = 0; i < chosen.Length; i++)
        {
            SerialRepair repair = chosen[i];
            if (repair.Kind != SerialRepairKind.Bind && repair.Kind != SerialRepairKind.Rebind) continue;

            int stepId = diagnosis.Errors[i].StepId;
            int index = IndexOfStep(stepId);
            if (index < 0 || changedParameters.Contains((stepId, repair.ParameterIndex)))
            {
                dropped.Add(i);
                continue;
            }

            SerialStep step = _steps[index];
            Constant?[] trial = step.Binding.ToArray();
            trial[repair.ParameterIndex] = repair.Object;
            if (!StaticFilter.Passes(Problem, step.Schema, trial))
            {
                dropped.Add(i);
                continue;
            }

            _steps[index] = step.WithBinding(repair.ParameterIndex, repair.Object!, Problem);
            changedParameters.Add((stepId, repair.ParameterIndex));
            applied++;
        }

        var created = new List<int>();
        var insertedCandidates = new HashSet<(int FlaggedStepId, GhostStep Candidate)>();
        for (int i = 0; i < chosen.Length; i++)
        {
            if (chosen[i].Kind != SerialRepairKind.Insert) continue;

            SerialError error = diagnosis.Errors[i];
            int position = error.Kind == SerialErrorKind.UnmetGoal ? _steps.Count : IndexOfStep(error.StepId);
            if (position < 0)
            {
                dropped.Add(i);
                continue;
            }

            applied++;
            // Errors of one step that choose the same candidate share the step it creates.
            GhostStep candidate = diagnosis.Candidates[chosen[i].CandidateIndex];
            if (!insertedCandidates.Add((error.StepId, candidate))) continue;

            var step = new SerialStep(_nextStepId++, candidate.Schema, candidate.CopyBinding(), error.StepId, error.Literal!, Problem);
            _steps.Insert(position, step);
            created.Add(step.Id);
        }

        Round++;
        _version++;
        _diagnosis = null;
        dropped.Sort();
        return new SerialEditResult(applied, dropped, created, deleted);
    }

    private int IndexOfStep(int id) => _steps.FindIndex(step => step.Id == id);
}
