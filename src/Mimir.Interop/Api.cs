using System;
using System.Collections.Concurrent;
using System.Runtime.InteropServices;

namespace Mimir.Interop;

// =============================================================================
// Mimir native interop layer
// =============================================================================
//
// This assembly is published with NativeAOT (PublishAot=true). Every method
// decorated with [UnmanagedCallersOnly] becomes a C-callable export in the
// resulting .so/.dll/.dylib, consumed from Python via ctypes.
//
// Conventions used throughout Api.*.cs partial files:
//
// * Handles:   Managed C# objects are stored in ObjectRegistry and returned to
//              Python as 32-bit integer handles. Handle 0 == null. Python is
//              responsible for calling mimir_free_handle on every non-zero
//              handle it receives.
//
// * Collections: Exposed as count + index pair. e.g.
//                  mimir_X_get_Y_count(handle)        -> int count
//                  mimir_X_get_Y(handle, int index)   -> int handle
//                Negative index or out-of-range returns 0.
//
// * Strings:   Returned as UTF-8 pointers allocated via Marshal.StringToCoTaskMemUTF8.
//              The caller must free them with mimir_free_string. Inputs are
//              UTF-8 IntPtrs decoded with Marshal.PtrToStringUTF8.
//
// * Booleans:  Returned as int (0 = false, 1 = true, -1 = invalid input)
//              when an operation can reject otherwise valid handles.
//
// * Errors:    Handle-returning factories return 0 when a precondition fails.
//              Boolean relationships return -1 for invalid inputs. Unexpected
//              exceptions propagate out of the [UnmanagedCallersOnly]
//              boundary, which in NativeAOT terminates the process with a
//              stack trace — that is the intended fail-fast behavior.
//
//              Non-handle accessors (counts, costs, hashes, strings)
//              treat a stale/invalid handle as a contract violation and throw
//              via Require&lt;T&gt;; previously they silently returned 0 / -1 / "".
//
// * Polarity:  Encoded as byte (0 = positive, 1 = negative).
// =============================================================================

internal static class ObjectRegistry
{
    private static readonly ConcurrentDictionary<int, object> _objects = new();
    private static int _nextHandle = 0;

    public static int Store(object? obj)
    {
        if (obj == null) return 0;
        if (obj is Mimir.Core.Grounding.State)
            throw new InvalidOperationException(
                "Compact State values cannot be registered as native handles.");

        while (true)
        {
            int handle = System.Threading.Interlocked.Increment(ref _nextHandle);
            if (handle == 0) continue;
            if (_objects.TryAdd(handle, obj)) return handle;
        }
    }

    public static T? Get<T>(int id) where T : class
    {
        if (id == 0) return null;
        return _objects.TryGetValue(id, out var obj) ? obj as T : null;
    }

    public static object? GetRaw(int id)
    {
        if (id == 0) return null;
        return _objects.TryGetValue(id, out var obj) ? obj : null;
    }

    public static void Release(int id)
    {
        if (id == 0) return;
        _objects.TryRemove(id, out _);
    }
}

public static partial class Exports
{
    internal const int InvalidBoolean = -1;

    internal enum NativeErrorCode
    {
        None = 0,
        InvalidHandle = 1,
        InvalidArgument = 2,
        InvalidOperation = 3,
        PddlLoad = 4,
        CallbackAborted = 5,
        Unexpected = 6,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct NativeError
    {
        public int Code;
        public int DocumentType;
        public int ErrorCode;
        public int Offset;
        public int Length;
        public int Line;
        public int Column;
        public IntPtr Message;
        public IntPtr SourcePath;
    }

    private sealed record NativeErrorData(
        NativeErrorCode Code,
        string Message,
        int DocumentType = -1,
        int ErrorCode = -1,
        int Offset = -1,
        int Length = -1,
        int Line = -1,
        int Column = -1,
        string? SourcePath = null);

    [ThreadStatic]
    private static NativeErrorData? _lastError;

    [UnmanagedCallersOnly(EntryPoint = "mimir_abi_version")]
    public static int AbiVersion() => 12;

    [UnmanagedCallersOnly(EntryPoint = "mimir_take_last_error")]
    public static unsafe byte TakeLastError(IntPtr errorPtr)
    {
        if (errorPtr == IntPtr.Zero || _lastError == null) return 0;

        NativeErrorData error = _lastError;
        _lastError = null;
        var nativeError = new NativeError
        {
            Code = (int)error.Code,
            DocumentType = error.DocumentType,
            ErrorCode = error.ErrorCode,
            Offset = error.Offset,
            Length = error.Length,
            Line = error.Line,
            Column = error.Column,
            Message = AllocUtf8(error.Message),
            SourcePath = AllocUtf8(error.SourcePath),
        };
        *(NativeError*)errorPtr = nativeError;
        return 1;
    }

    internal static TResult ReadValue<T, TResult>(
        int handle,
        TResult errorValue,
        Func<T, TResult> read)
        where T : class
    {
        T? value = ObjectRegistry.Get<T>(handle);
        if (value == null)
        {
            RecordInvalidHandle(typeof(T).Name, handle);
            return errorValue;
        }

        try
        {
            return read(value);
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return errorValue;
        }
    }

    internal static int CreateHandle(Func<object> factory)
    {
        try
        {
            return ObjectRegistry.Store(factory());
        }
        catch (ArgumentException exception)
        {
            RecordError(exception);
            return 0;
        }
        catch (InvalidOperationException exception)
        {
            RecordError(exception);
            return 0;
        }
        catch (Mimir.Core.Schemas.PddlLoadException exception)
        {
            RecordError(exception);
            return 0;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return 0;
        }
    }

    internal static int EvaluateRelationship(Func<bool> relationship)
    {
        try
        {
            return relationship() ? 1 : 0;
        }
        catch (ArgumentException exception)
        {
            RecordError(exception);
            return InvalidBoolean;
        }
        catch (InvalidOperationException exception)
        {
            RecordError(exception);
            return InvalidBoolean;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return InvalidBoolean;
        }
    }

    internal static void RecordInvalidHandle(string typeName, int handle)
        => _lastError = new NativeErrorData(
            NativeErrorCode.InvalidHandle,
            $"invalid {typeName} handle: {handle}");

    internal static void RecordError(Exception exception)
    {
        _lastError = exception switch
        {
            Mimir.Core.Schemas.PddlLoadException pddl => new NativeErrorData(
                NativeErrorCode.PddlLoad,
                pddl.Message,
                (int)pddl.DocumentType,
                (int)pddl.ErrorCode,
                pddl.Span?.Offset ?? -1,
                pddl.Span?.Length ?? -1,
                pddl.Span?.Line ?? -1,
                pddl.Span?.Column ?? -1,
                pddl.SourcePath),
            ArgumentException => new NativeErrorData(
                NativeErrorCode.InvalidArgument,
                exception.Message),
            InvalidOperationException => new NativeErrorData(
                NativeErrorCode.InvalidOperation,
                exception.Message),
            _ when exception.GetType().Name == "CallbackAbortedException" => new NativeErrorData(
                NativeErrorCode.CallbackAborted,
                exception.Message),
            _ => new NativeErrorData(
                NativeErrorCode.Unexpected,
                exception.Message),
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_value_equals")]
    public static int ValueEquals(int leftHandle, int rightHandle)
    {
        object? left = ObjectRegistry.GetRaw(leftHandle);
        object? right = ObjectRegistry.GetRaw(rightHandle);
        if (left == null || right == null) return InvalidBoolean;
        return left.Equals(right) ? 1 : 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_value_hash")]
    public static byte ValueHash(int handle, IntPtr resultPtr)
    {
        object? value = ObjectRegistry.GetRaw(handle);
        if (value == null || resultPtr == IntPtr.Zero) return 0;
        Marshal.WriteInt32(resultPtr, value.GetHashCode());
        return 1;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_free_handle")]
    public static void FreeHandle(int handle)
    {
        ObjectRegistry.Release(handle);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_free_string")]
    public static void FreeString(IntPtr ptr)
    {
        if (ptr != IntPtr.Zero)
        {
            Marshal.FreeCoTaskMem(ptr);
        }
    }

    internal static IntPtr AllocUtf8(string? value)
    {
        if (value == null) return IntPtr.Zero;
        return Marshal.StringToCoTaskMemUTF8(value);
    }

    internal static string? ReadUtf8(IntPtr ptr)
    {
        if (ptr == IntPtr.Zero) return null;
        return Marshal.PtrToStringUTF8(ptr);
    }
}
