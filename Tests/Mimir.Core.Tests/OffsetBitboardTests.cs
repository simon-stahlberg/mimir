using Mimir.Core.Grounding;

namespace Mimir.Core.Tests;

public class OffsetBitboardTests
{
    [Fact]
    public void FromDense_CopiesInputAndPreservesSetBits()
    {
        ulong[] source = [0UL, 0UL, 8UL, 0UL, 2UL, 0UL];

        OffsetBitboard bitboard = OffsetBitboard.FromDense(source);
        source[2] = 0UL;
        source[4] = 0UL;

        Assert.Equal(new[] { 131, 257 }, BitboardOps.DecodeSetBits(bitboard));
    }

    [Fact]
    public void EmptyRepresentationsAreCanonicalAndHaveNoSetBits()
    {
        OffsetBitboard denseEmpty = OffsetBitboard.FromDense([0UL, 0UL]);
        OffsetBitboard indexedEmpty = OffsetBitboard.FromSetBitIndices(Array.Empty<int>());
        OffsetBitboard defaultEmpty = default;

        Assert.Equal(defaultEmpty, denseEmpty);
        Assert.Equal(defaultEmpty, indexedEmpty);
        Assert.Equal(defaultEmpty.GetHashCode(), denseEmpty.GetHashCode());
        Assert.True(denseEmpty.IsEmpty);
        Assert.Equal(0, BitboardOps.PopCount(denseEmpty));
        Assert.Empty(BitboardOps.DecodeSetBits(denseEmpty));
        Assert.Equal(-1, BitboardOps.NthSetBitIndex(denseEmpty, 0));
    }

    [Fact]
    public void SetBitOperationsPreserveGlobalIndicesAcrossInternalZeroWords()
    {
        OffsetBitboard bitboard = OffsetBitboard.FromSetBitIndices([65, 257, 65]);

        Assert.Equal(2, BitboardOps.PopCount(bitboard));
        Assert.Equal(new[] { 65, 257 }, BitboardOps.DecodeSetBits(bitboard));
        Assert.Equal(-1, BitboardOps.NthSetBitIndex(bitboard, -1));
        Assert.Equal(65, BitboardOps.NthSetBitIndex(bitboard, 0));
        Assert.Equal(257, BitboardOps.NthSetBitIndex(bitboard, 1));
        Assert.Equal(-1, BitboardOps.NthSetBitIndex(bitboard, 2));
    }

    [Fact]
    public void EqualGlobalMasksHaveEqualHashes()
    {
        OffsetBitboard dense = OffsetBitboard.FromDense([0UL, 1UL, 0UL, 2UL, 0UL]);
        OffsetBitboard indexed = OffsetBitboard.FromSetBitIndices([64, 193]);

        Assert.Equal(dense, indexed);
        Assert.True(dense == indexed);
        Assert.False(dense != indexed);
        Assert.Equal(dense.GetHashCode(), indexed.GetHashCode());
        Assert.Single(new HashSet<OffsetBitboard> { dense, indexed });
    }

    [Fact]
    public void FactoriesRejectInvalidInputs()
    {
        Assert.Throws<ArgumentNullException>(() => OffsetBitboard.FromDense(null!));
        Assert.Throws<ArgumentNullException>(() => OffsetBitboard.FromSetBitIndices(null!));
        Assert.Throws<ArgumentOutOfRangeException>(() => OffsetBitboard.FromSetBitIndices([-1]));
        Assert.Throws<ArgumentNullException>(() => BitboardOps.PopCount((ulong[])null!));
        Assert.Throws<ArgumentNullException>(() => BitboardOps.DecodeSetBits(null!));
        Assert.Throws<ArgumentNullException>(() => BitboardOps.NthSetBitIndex(null!, 0));
    }
}
