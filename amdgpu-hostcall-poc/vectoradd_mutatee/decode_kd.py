import sys, struct

def read_kd_rsrc1(path, kdname):
    d = open(path,'rb').read()
    e_shoff = struct.unpack_from('<Q', d, 0x28)[0]
    e_shentsize = struct.unpack_from('<H', d, 0x3a)[0]
    e_shnum = struct.unpack_from('<H', d, 0x3c)[0]
    secs = []
    for i in range(e_shnum):
        b = e_shoff + i*e_shentsize
        sh_type = struct.unpack_from('<I', d, b+4)[0]
        sh_addr = struct.unpack_from('<Q', d, b+16)[0]
        sh_off  = struct.unpack_from('<Q', d, b+24)[0]
        sh_size = struct.unpack_from('<Q', d, b+32)[0]
        sh_link = struct.unpack_from('<I', d, b+40)[0]
        sh_ent  = struct.unpack_from('<Q', d, b+56)[0]
        secs.append((sh_type,sh_addr,sh_off,sh_size,sh_link,sh_ent))
    # find symtab (2) or dynsym (11)
    sym = next((s for s in secs if s[0]==2), None) or next((s for s in secs if s[0]==11), None)
    _,_,soff,ssize,slink,sent = sym
    stroff = secs[slink][2]
    for o in range(soff, soff+ssize, sent):
        st_name = struct.unpack_from('<I', d, o)[0]
        st_shndx= struct.unpack_from('<H', d, o+6)[0]
        st_value= struct.unpack_from('<Q', d, o+8)[0]
        end = d.index(b'\0', stroff+st_name)
        name = d[stroff+st_name:end].decode('latin1')
        if name == kdname:
            sec = secs[st_shndx]
            foff = sec[2] + (st_value - sec[1])
            kd = d[foff:foff+64]
            rsrc1 = struct.unpack_from('<I', kd, 48)[0]
            priv  = struct.unpack_from('<I', kd, 4)[0]
            return rsrc1, rsrc1 & 0x3f, (rsrc1>>6)&0xf, priv
    return None

kdname = sys.argv[1]
for p in sys.argv[2:]:
    r = read_kd_rsrc1(p, kdname)
    if not r: print(f"{p}: '{kdname}' not found"); continue
    rsrc1,vg,sg,priv = r
    print(f"{p}\n   rsrc1=0x{rsrc1:08x}  VGPR_gran={vg} => {(vg+1)*4} vgprs   SGPR_gran={sg}   priv_seg={priv}")
