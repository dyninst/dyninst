#!/usr/bin/env python3
# sync_note_from_kd.py — make an instrumented AMDGPU code object's NT_AMDGPU_METADATA
# note agree with its (dyninst-bumped) kernel descriptors.
#
# Dyninst bumps each kernel's KD (private_segment_fixed_size, granted VGPR/SGPR) when
# it inserts calls, but leaves the msgpack `.note` stale. HIP/ROCr fills the AQL
# dispatch packet's private_segment_size from the NOTE, so a stale note => packet says
# 0 scratch => GPU faults => ROCr's scratch-recovery finds no scratch-declaring packet
# and NULL-derefs (amd_aql_queue.cpp HandleInsufficientScratch). Syncing the note to the
# KD fixes it. (The dedicated launcher dodged this by reading private_segment_size from
# the KD symbol directly.)
#
# Flow mirrors the proven KR-13 tooling: llvm-objcopy dump/remove/add the .note, rewrite
# the msgpack + note header, then repoint the PT_NOTE program header at the new bytes.
#
# Usage: sync_note_from_kd.py <inst.co> [--objcopy PATH]
import sys, os, struct, subprocess, tempfile
import msgpack

def parse_elf_kds(path):
    """Return {kernel_name: (private_seg, vgpr_count, sgpr_count)} read from each *.kd."""
    d = open(path, 'rb').read()
    e_shoff = struct.unpack_from('<Q', d, 0x28)[0]
    e_shentsize = struct.unpack_from('<H', d, 0x3a)[0]
    e_shnum = struct.unpack_from('<H', d, 0x3c)[0]
    secs = []
    for i in range(e_shnum):
        b = e_shoff + i*e_shentsize
        secs.append((struct.unpack_from('<I', d, b+4)[0],   # type
                     struct.unpack_from('<Q', d, b+16)[0],  # addr
                     struct.unpack_from('<Q', d, b+24)[0],  # off
                     struct.unpack_from('<Q', d, b+32)[0],  # size
                     struct.unpack_from('<I', d, b+40)[0],  # link
                     struct.unpack_from('<Q', d, b+56)[0])) # entsize
    sym = next((s for s in secs if s[0]==2), None) or next((s for s in secs if s[0]==11), None)
    _,_,soff,ssize,slink,sent = sym
    stroff = secs[slink][2]
    out = {}
    for o in range(soff, soff+ssize, sent):
        st_name = struct.unpack_from('<I', d, o)[0]
        st_shndx= struct.unpack_from('<H', d, o+6)[0]
        st_value= struct.unpack_from('<Q', d, o+8)[0]
        end = d.index(b'\0', stroff+st_name)
        name = d[stroff+st_name:end].decode('latin1')
        if not name.endswith('.kd') or st_shndx >= len(secs):
            continue
        sec = secs[st_shndx]
        foff = sec[2] + (st_value - sec[1])
        kd = d[foff:foff+64]
        priv  = struct.unpack_from('<I', kd, 4)[0]     # PRIVATE_SEGMENT_FIXED_SIZE
        rsrc1 = struct.unpack_from('<I', kd, 48)[0]    # COMPUTE_PGM_RSRC1
        vg = rsrc1 & 0x3f                              # GRANULATED_WORKITEM_VGPR_COUNT
        sg = (rsrc1 >> 6) & 0xf                        # GRANULATED_WAVEFRONT_SGPR_COUNT
        vgpr_count = (vg + 1) * 4                      # gfx9 wave64 VGPR granule 4
        sgpr_count = (sg + 1) * 8                      # amdhsa SGPR granule 8 (over-estimate, safe)
        out[name[:-3]] = (priv, vgpr_count, sgpr_count)   # strip ".kd"
    return out

def rewrite_note(note_bytes, kd_map):
    """Parse the AMDGPU note, sync per-kernel resource fields, return new note bytes."""
    name_sz, desc_sz, ntype = struct.unpack_from('<III', note_bytes, 0)
    name = note_bytes[12:12+name_sz]
    off = 12 + name_sz
    while off % 4: off += 1
    root = msgpack.unpackb(note_bytes[off:off+desc_sz], strict_map_key=False, raw=False)

    changed = 0
    for k in root.get('amdhsa.kernels', []):
        kn = k.get('.name')
        if kn in kd_map:
            priv, vgpr, sgpr = kd_map[kn]
            for key, val in (('.private_segment_fixed_size', priv),
                             ('.vgpr_count', vgpr), ('.sgpr_count', sgpr)):
                if k.get(key) != val:
                    print(f"  {kn}: {key} {k.get(key)} -> {val}")
                    k[key] = val
                    changed += 1
    print(f"  ({changed} field(s) updated)")

    new_desc = msgpack.packb(root, use_bin_type=True)
    out = bytearray()
    out += struct.pack('<III', name_sz, len(new_desc), ntype)
    out += name
    while len(out) % 4: out.append(0)
    out += new_desc
    while len(out) % 4: out.append(0)
    return bytes(out)

def repoint_pt_note(path):
    """Set the PT_NOTE program header offset/size to the (relocated) .note section."""
    with open(path, 'r+b') as f:
        d = bytearray(f.read())
        e_phoff  = struct.unpack_from('<Q', d, 0x20)[0]
        e_shoff  = struct.unpack_from('<Q', d, 0x28)[0]
        e_phentsize = struct.unpack_from('<H', d, 0x36)[0]
        e_phnum  = struct.unpack_from('<H', d, 0x38)[0]
        e_shentsize = struct.unpack_from('<H', d, 0x3a)[0]
        e_shnum  = struct.unpack_from('<H', d, 0x3c)[0]
        e_shstrndx = struct.unpack_from('<H', d, 0x3e)[0]
        # locate .note section
        shstr_off = struct.unpack_from('<Q', d, e_shoff + e_shstrndx*e_shentsize + 24)[0]
        note_off = note_size = None
        for i in range(e_shnum):
            b = e_shoff + i*e_shentsize
            nameoff = struct.unpack_from('<I', d, b+0)[0]
            end = d.index(b'\0', shstr_off+nameoff)
            if d[shstr_off+nameoff:end] == b'.note':
                note_off  = struct.unpack_from('<Q', d, b+24)[0]
                note_size = struct.unpack_from('<Q', d, b+32)[0]
                break
        if note_off is None:
            print("  WARN: no .note section after add; PT_NOTE not repointed"); return
        # patch the PT_NOTE phdr
        for i in range(e_phnum):
            pb = e_phoff + i*e_phentsize
            if struct.unpack_from('<I', d, pb)[0] == 4:  # PT_NOTE
                struct.pack_into('<Q', d, pb+8,  note_off)   # p_offset
                struct.pack_into('<Q', d, pb+16, note_off)   # p_vaddr
                struct.pack_into('<Q', d, pb+24, note_off)   # p_paddr
                struct.pack_into('<Q', d, pb+32, note_size)  # p_filesz
                struct.pack_into('<Q', d, pb+40, note_size)  # p_memsz
                print(f"  PT_NOTE -> offset={note_off:#x} size={note_size:#x}")
                break
        f.seek(0); f.write(d)

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    objcopy = '/opt/rocm-7.0.2/lib/llvm/bin/llvm-objcopy'
    for i,a in enumerate(sys.argv):
        if a == '--objcopy': objcopy = sys.argv[i+1]
    co = args[0]

    kd_map = parse_elf_kds(co)
    print(f"[sync-note] KD resource usage in {co}:")
    for kn,(p,v,s) in kd_map.items():
        print(f"  {kn}: private_seg={p} vgpr={v} sgpr={s}")

    with tempfile.TemporaryDirectory() as td:
        note = os.path.join(td, 'note.bin')
        subprocess.check_call([objcopy, f'--dump-section=.note={note}', co])
        new = rewrite_note(open(note,'rb').read(), kd_map)
        newf = os.path.join(td, 'note.new')
        open(newf,'wb').write(new)
        subprocess.check_call([objcopy, '--remove-section=.note', co])
        subprocess.check_call([objcopy, f'--add-section=.note={newf}', co])
        repoint_pt_note(co)
    print(f"[sync-note] {co}: note synced to KD")

if __name__ == '__main__':
    main()
