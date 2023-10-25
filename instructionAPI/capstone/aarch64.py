class aarch64:

    def __init__(self):
        self.aliasMap = {}
        self.capstone_prefix = "ARM64"
        self.dyninst_prefix = "aarch64_op"
        self.dyninst_register_prefix="aarch64"
