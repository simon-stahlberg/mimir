namespace Mimir.Core.Engines;

using Grounding;
using Schemas;
using System.Runtime.CompilerServices;

internal readonly struct DerivedFactKey : IEquatable<DerivedFactKey>
{
    private readonly Predicate<Derived> _predicate;
    private readonly int _arity;
    private readonly Constant? _arg1;
    private readonly Constant? _arg2;
    private readonly Constant? _arg3;
    private readonly Constant? _arg4;
    private readonly Constant[]? _extraArguments;

    internal Predicate<Derived> Predicate => _predicate;

    internal DerivedFactKey(Predicate<Derived> predicate)
    {
        _predicate = predicate;
        _arity = 0;
        _arg1 = null;
        _arg2 = null;
        _arg3 = null;
        _arg4 = null;
        _extraArguments = null;
    }

    internal DerivedFactKey(Predicate<Derived> predicate, Constant arg1)
    {
        _predicate = predicate;
        _arity = 1;
        _arg1 = arg1;
        _arg2 = null;
        _arg3 = null;
        _arg4 = null;
        _extraArguments = null;
    }

    internal DerivedFactKey(
        Predicate<Derived> predicate,
        Constant arg1,
        Constant arg2)
    {
        _predicate = predicate;
        _arity = 2;
        _arg1 = arg1;
        _arg2 = arg2;
        _arg3 = null;
        _arg4 = null;
        _extraArguments = null;
    }

    internal DerivedFactKey(
        Predicate<Derived> predicate,
        Constant arg1,
        Constant arg2,
        Constant arg3)
    {
        _predicate = predicate;
        _arity = 3;
        _arg1 = arg1;
        _arg2 = arg2;
        _arg3 = arg3;
        _arg4 = null;
        _extraArguments = null;
    }

    internal DerivedFactKey(
        Predicate<Derived> predicate,
        Constant arg1,
        Constant arg2,
        Constant arg3,
        Constant arg4)
    {
        _predicate = predicate;
        _arity = 4;
        _arg1 = arg1;
        _arg2 = arg2;
        _arg3 = arg3;
        _arg4 = arg4;
        _extraArguments = null;
    }

    internal DerivedFactKey(
        Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments)
    {
        _predicate = predicate;
        _arity = arguments.Count;
        _arg1 = arguments.Count > 0 ? arguments[0] : null;
        _arg2 = arguments.Count > 1 ? arguments[1] : null;
        _arg3 = arguments.Count > 2 ? arguments[2] : null;
        _arg4 = arguments.Count > 3 ? arguments[3] : null;

        if (arguments.Count > 4)
        {
            _extraArguments = new Constant[arguments.Count - 4];
            for (int i = 4; i < arguments.Count; i++)
                _extraArguments[i - 4] = arguments[i];
        }
        else
        {
            _extraArguments = null;
        }
    }

    internal DerivedFactKey(
        Predicate<Derived> predicate,
        Constant[] arguments,
        int argumentCount)
    {
        if (argumentCount < 0 || argumentCount > arguments.Length)
            throw new ArgumentOutOfRangeException(nameof(argumentCount));

        _predicate = predicate;
        _arity = argumentCount;
        _arg1 = argumentCount > 0 ? arguments[0] : null;
        _arg2 = argumentCount > 1 ? arguments[1] : null;
        _arg3 = argumentCount > 2 ? arguments[2] : null;
        _arg4 = argumentCount > 3 ? arguments[3] : null;

        if (argumentCount > 4)
        {
            _extraArguments = new Constant[argumentCount - 4];
            Array.Copy(arguments, 4, _extraArguments, 0, argumentCount - 4);
        }
        else
        {
            _extraArguments = null;
        }
    }

    internal Constant GetArgument(int index)
    {
        if (index < 0 || index >= _arity)
            throw new ArgumentOutOfRangeException(nameof(index));

        return index switch
        {
            0 => _arg1!,
            1 => _arg2!,
            2 => _arg3!,
            3 => _arg4!,
            _ => _extraArguments![index - 4],
        };
    }

    internal Constant[] GetArguments()
    {
        var arguments = new Constant[_arity];
        for (int i = 0; i < arguments.Length; i++)
            arguments[i] = GetArgument(i);
        return arguments;
    }

    public bool Equals(DerivedFactKey other)
    {
        if (!ReferenceEquals(_predicate, other._predicate))
            return false;
        if (_arity != other._arity)
            return false;
        if (!ReferenceEquals(_arg1, other._arg1))
            return false;
        if (!ReferenceEquals(_arg2, other._arg2))
            return false;
        if (!ReferenceEquals(_arg3, other._arg3))
            return false;
        if (!ReferenceEquals(_arg4, other._arg4))
            return false;

        if (_extraArguments is null)
            return other._extraArguments is null;
        if (other._extraArguments is null
            || _extraArguments.Length != other._extraArguments.Length)
        {
            return false;
        }

        for (int i = 0; i < _extraArguments.Length; i++)
        {
            if (!ReferenceEquals(_extraArguments[i], other._extraArguments[i]))
                return false;
        }

        return true;
    }

    public override bool Equals(object? obj)
        => obj is DerivedFactKey other && Equals(other);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(_predicate));
        hash.Add(_arity);
        hash.Add(_arg1 is null ? 0 : RuntimeHelpers.GetHashCode(_arg1));
        hash.Add(_arg2 is null ? 0 : RuntimeHelpers.GetHashCode(_arg2));
        hash.Add(_arg3 is null ? 0 : RuntimeHelpers.GetHashCode(_arg3));
        hash.Add(_arg4 is null ? 0 : RuntimeHelpers.GetHashCode(_arg4));
        if (_extraArguments is not null)
        {
            for (int i = 0; i < _extraArguments.Length; i++)
                hash.Add(RuntimeHelpers.GetHashCode(_extraArguments[i]));
        }

        return hash.ToHashCode();
    }
}

internal enum DerivedTruth : byte
{
    Unknown,
    False,
    True,
}

internal sealed class DerivedEvaluationWorkspace
{
    private readonly List<DerivedEvaluationFrame> _frames = new();
    private int _depth;

    internal DerivedEvaluationFrame RentFrame(
        int bindingCount,
        int headArgumentCount)
    {
        DerivedEvaluationFrame frame;
        if (_depth == _frames.Count)
        {
            frame = new DerivedEvaluationFrame();
            _frames.Add(frame);
        }
        else
        {
            frame = _frames[_depth];
        }

        frame.Prepare(bindingCount, headArgumentCount);
        _depth++;
        return frame;
    }

    internal void ReturnFrame(DerivedEvaluationFrame frame)
    {
        if (_depth == 0 || !ReferenceEquals(_frames[_depth - 1], frame))
        {
            throw new InvalidOperationException(
                "Derived evaluation frames must be returned in LIFO order.");
        }

        _depth--;
        frame.Clear();
    }

    internal void PrepareForReturn()
    {
        if (_depth != 0)
        {
            throw new InvalidOperationException(
                "A derived evaluation workspace cannot be returned with active frames.");
        }
    }
}

internal sealed class DerivedEvaluationFrame
{
    private int _bindingCount;
    private int _headArgumentCount;
    private Constant?[] _bindings = Array.Empty<Constant?>();
    private Constant[] _headArguments = Array.Empty<Constant>();

    internal Constant?[] Bindings => _bindings;
    internal Constant[] HeadArguments => _headArguments;

    internal void Prepare(int bindingCount, int headArgumentCount)
    {
        if (_bindings.Length < bindingCount)
            Array.Resize(ref _bindings, bindingCount);
        if (_headArguments.Length < headArgumentCount)
            Array.Resize(ref _headArguments, headArgumentCount);

        _bindingCount = bindingCount;
        _headArgumentCount = headArgumentCount;
    }

    internal void Clear()
    {
        Array.Clear(_bindings, 0, _bindingCount);
        Array.Clear(_headArguments, 0, _headArgumentCount);
        _bindingCount = 0;
        _headArgumentCount = 0;
    }
}

internal sealed class DerivedPredicateEvaluation
{
    private readonly DerivedPredicateClosure _closure;
    private DerivedTruth[] _truth;

    internal State State { get; }

    internal DerivedPredicateEvaluation(
        DerivedPredicateClosure closure,
        State state,
        int initialSlotCount)
    {
        _closure = closure;
        State = state;
        _truth = initialSlotCount == 0
            ? Array.Empty<DerivedTruth>()
            : new DerivedTruth[initialSlotCount];
    }

    internal void CompleteRecursiveComponents()
        => _closure.CompleteRecursiveComponents(this);

    internal bool IsTrue(
        Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments)
        => _closure.IsStateDependentTrue(
            this,
            predicate,
            new DerivedFactKey(predicate, arguments));

    internal bool IsTrue(
        Predicate<Derived> predicate,
        DerivedEvaluationSlot slot)
        => _closure.IsStateDependentTrue(this, predicate, slot);

    internal bool TryGetAcyclicTruth(
        DerivedEvaluationSlot slot,
        out bool isTrue)
    {
        EnsureSlot(slot);
        DerivedTruth truth = _truth[slot.Value];
        isTrue = truth == DerivedTruth.True;
        return truth != DerivedTruth.Unknown;
    }

    internal void SetAcyclicTruth(DerivedEvaluationSlot slot, bool isTrue)
    {
        EnsureSlot(slot);
        if (_truth[slot.Value] != DerivedTruth.Unknown)
            throw new InvalidOperationException(nameof(slot));

        _truth[slot.Value] = isTrue ? DerivedTruth.True : DerivedTruth.False;
    }

    internal bool IsRecursiveTrue(DerivedEvaluationSlot slot)
    {
        EnsureSlot(slot);
        return _truth[slot.Value] == DerivedTruth.True;
    }

    internal bool AddRecursiveTruth(DerivedEvaluationSlot slot)
    {
        EnsureSlot(slot);
        if (_truth[slot.Value] == DerivedTruth.True)
            return false;
        if (_truth[slot.Value] == DerivedTruth.False)
            throw new InvalidOperationException(nameof(slot));

        _truth[slot.Value] = DerivedTruth.True;
        return true;
    }

    private void EnsureSlot(DerivedEvaluationSlot slot)
    {
        if (slot.Value < 0)
            throw new ArgumentOutOfRangeException(nameof(slot));
        if (slot.Value < _truth.Length)
            return;

        int newLength = _truth.Length == 0 ? 4 : _truth.Length * 2;
        if (newLength <= slot.Value)
            newLength = slot.Value + 1;
        Array.Resize(ref _truth, newLength);
    }

}
