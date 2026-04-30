import lldb


def PhysicalAddress_summary(valobj, internal_dict):
    addr = valobj.GetChildMemberWithName('addr').GetValueAsUnsigned(0)
    return f'0x{addr:08x}'


def VirtualAddress_summary(valobj, internal_dict):
    addr = valobj.GetChildMemberWithName('addr').GetValueAsUnsigned(0)
    return f'0x{addr:08x}'


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        'type summary add -F lldb_nyan.PhysicalAddress_summary "nyan::paging::PhysicalAddress"'
    )
    debugger.HandleCommand(
        'type summary add -F lldb_nyan.VirtualAddress_summary "nyan::paging::VirtualAddress"'
    )
    print("[nyan] LLDB formatters loaded: PhysicalAddress, VirtualAddress")
