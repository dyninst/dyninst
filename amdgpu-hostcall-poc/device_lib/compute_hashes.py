#!/usr/bin/env python3
"""
Compute ELF and GNU hash values for updating .hash and .gnu.hash sections
when adding or modifying symbols in a code object's dynamic symbol table.

Usage: edit the 'symbols' list and 'nbucket' / 'nbuckets_gnu' as needed,
       then run to get the correct Bucket/Chain/.gnu.hash values for the YAML.
"""

def elf_hash(name):
    h = 0
    for c in name:
        h = ((h << 4) + ord(c)) & 0xFFFFFFFF
        g = h & 0xF0000000
        if g:
            h ^= g >> 24
        h &= ~g & 0xFFFFFFFF
    return h

def gnu_hash(name):
    h = 5381
    for c in name:
        h = ((h << 5) + h + ord(c)) & 0xFFFFFFFF
    return h

def build_elf_hash(symbols_from_1, nbucket):
    """
    Build .hash Bucket and Chain arrays.
    symbols_from_1: list of symbol names starting at dynsym index 1.
    Returns (buckets, chain) where len(chain) = 1 + len(symbols_from_1).
    chain[0] is always 0 (the undefined symbol at index 0).
    """
    n = len(symbols_from_1)
    buckets = [0] * nbucket
    chain   = [0] * (n + 1)   # index 0 = undef (always 0)

    for i, name in enumerate(symbols_from_1, start=1):
        b = elf_hash(name) % nbucket
        chain[i] = buckets[b]
        buckets[b] = i

    return buckets, chain

def build_gnu_hash(symbols_from_symndx, nbuckets_gnu, sym_ndx, shift2=26):
    """
    Build .gnu.hash BloomFilter, HashBuckets, HashValues.
    symbols_from_symndx: list of symbol names starting at dynsym index sym_ndx.
    """
    bloom_word_bits = 64
    bloom = 0
    hashes = [gnu_hash(n) for n in symbols_from_symndx]

    for h in hashes:
        bloom |= (1 << (h % bloom_word_bits))
        bloom |= (1 << ((h >> shift2) % bloom_word_bits))
    bloom &= (1 << bloom_word_bits) - 1

    # Bucket: stores dynsym index of first symbol in that bucket
    gnu_buckets = [0] * nbuckets_gnu
    for idx, h in enumerate(hashes):
        b = h % nbuckets_gnu
        dynsym_idx = sym_ndx + idx
        if gnu_buckets[b] == 0 or gnu_buckets[b] > dynsym_idx:
            gnu_buckets[b] = dynsym_idx

    # Hash values: one per symbol from sym_ndx, in dynsym order.
    # The lowest bit of each value indicates whether it is the LAST symbol
    # in its bucket (bit=1) or not (bit=0).
    hash_values = []
    for idx, h in enumerate(hashes):
        b = h % nbuckets_gnu
        dynsym_idx = sym_ndx + idx
        # Is this the last symbol in bucket b?
        is_last = True
        for j in range(idx + 1, len(hashes)):
            if hashes[j] % nbuckets_gnu == b:
                is_last = False
                break
        hv = (h & ~1) | (1 if is_last else 0)
        hash_values.append(hv)

    return bloom, gnu_buckets, hash_values


if __name__ == "__main__":
    # ------- configure here -------
    # dynsym layout:
    #   index 0: undefined (not hashed)
    #   index 1: __hip_cuid_db78d7753046f6bf
    #   index 2: funptr1
    all_hashed_symbols = [
        "__hip_cuid_db78d7753046f6bf",
        "funptr1",
    ]
    sym_ndx    = 1      # first hashed symbol index in dynsym
    nbucket    = 2      # .hash nbucket
    nbuckets_gnu = 1    # .gnu.hash nbuckets
    shift2     = 26
    # ------------------------------

    print("=== ELF hash (.hash section) ===")
    buckets, chain = build_elf_hash(all_hashed_symbols, nbucket)
    print(f"  Bucket: {buckets}")
    print(f"  Chain:  {chain}")
    print(f"  YAML:   Bucket: {buckets!r}")
    print(f"  YAML:   Chain:  {chain!r}")

    print()
    print("=== GNU hash (.gnu.hash section) ===")
    bloom, gnu_buckets, hash_values = build_gnu_hash(
        all_hashed_symbols, nbuckets_gnu, sym_ndx, shift2)
    print(f"  BloomFilter:  [{bloom:#x}]")
    print(f"  HashBuckets:  {[f'{b:#x}' for b in gnu_buckets]}")
    print(f"  HashValues:   {[f'{h:#010x}' for h in hash_values]}")
    print()
    print("  YAML snippet:")
    print(f"    Header:")
    print(f"      SymNdx:      {sym_ndx:#x}")
    print(f"      Shift2:      {shift2:#x}")
    print(f"    BloomFilter: [ {bloom:#x} ]")
    print(f"    HashBuckets: [ {', '.join(f'{b:#x}' for b in gnu_buckets)} ]")
    print(f"    HashValues:  [ {', '.join(f'{h:#010x}' for h in hash_values)} ]")
