#!/usr/bin/env python3
"""Create crackme.exe suspended so x32dbg can attach before it runs."""

from __future__ import annotations

import argparse
import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import subprocess


CREATE_SUSPENDED = 0x00000004
CREATE_NEW_CONSOLE = 0x00000010
INFINITE = 0xFFFFFFFF
ENTRY_POINT = 0x4012E0
CHECK_DEBUGGER = 0x401E9A
INLINE_DETECTED_RESULT = 0x4023B8

EXIT_HINTS = {
    0xC0000135: "missing DLL, usually libgcc_s_dw2-1.dll or libstdc++-6.dll",
    0xC000007B: "bad image format, usually a 32-bit/64-bit DLL mismatch",
}


class STARTUPINFO(ctypes.Structure):
    _fields_ = [
        ("cb", wintypes.DWORD),
        ("lpReserved", wintypes.LPWSTR),
        ("lpDesktop", wintypes.LPWSTR),
        ("lpTitle", wintypes.LPWSTR),
        ("dwX", wintypes.DWORD),
        ("dwY", wintypes.DWORD),
        ("dwXSize", wintypes.DWORD),
        ("dwYSize", wintypes.DWORD),
        ("dwXCountChars", wintypes.DWORD),
        ("dwYCountChars", wintypes.DWORD),
        ("dwFillAttribute", wintypes.DWORD),
        ("dwFlags", wintypes.DWORD),
        ("wShowWindow", wintypes.WORD),
        ("cbReserved2", wintypes.WORD),
        ("lpReserved2", ctypes.c_void_p),
        ("hStdInput", wintypes.HANDLE),
        ("hStdOutput", wintypes.HANDLE),
        ("hStdError", wintypes.HANDLE),
    ]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("hProcess", wintypes.HANDLE),
        ("hThread", wintypes.HANDLE),
        ("dwProcessId", wintypes.DWORD),
        ("dwThreadId", wintypes.DWORD),
    ]


kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
kernel32.CreateProcessW.argtypes = [
    wintypes.LPCWSTR,
    wintypes.LPWSTR,
    ctypes.c_void_p,
    ctypes.c_void_p,
    wintypes.BOOL,
    wintypes.DWORD,
    ctypes.c_void_p,
    wintypes.LPCWSTR,
    ctypes.POINTER(STARTUPINFO),
    ctypes.POINTER(PROCESS_INFORMATION),
]
kernel32.CreateProcessW.restype = wintypes.BOOL
kernel32.ResumeThread.argtypes = [wintypes.HANDLE]
kernel32.ResumeThread.restype = wintypes.DWORD
kernel32.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
kernel32.WaitForSingleObject.restype = wintypes.DWORD
kernel32.GetExitCodeProcess.argtypes = [wintypes.HANDLE, ctypes.POINTER(wintypes.DWORD)]
kernel32.GetExitCodeProcess.restype = wintypes.BOOL
kernel32.TerminateProcess.argtypes = [wintypes.HANDLE, wintypes.UINT]
kernel32.TerminateProcess.restype = wintypes.BOOL
kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
kernel32.CloseHandle.restype = wintypes.BOOL
kernel32.WriteProcessMemory.argtypes = [
    wintypes.HANDLE,
    ctypes.c_void_p,
    ctypes.c_void_p,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
kernel32.WriteProcessMemory.restype = wintypes.BOOL


def fail_last_error(message: str) -> None:
    code = ctypes.get_last_error()
    raise OSError(code, f"{message}: {ctypes.FormatError(code)}")


def process_exit_code(process_handle: wintypes.HANDLE) -> int:
    code = wintypes.DWORD()
    if not kernel32.GetExitCodeProcess(process_handle, ctypes.byref(code)):
        fail_last_error("GetExitCodeProcess failed")
    return int(code.value)


def print_exit_status(code: int) -> None:
    hint = EXIT_HINTS.get(code)
    print(f"Target exited: 0x{code:08X}")
    if hint:
        print(f"Hint: {hint}")


def write_memory(process_handle: wintypes.HANDLE, address: int, data: bytes) -> None:
    written = ctypes.c_size_t()
    buffer = ctypes.create_string_buffer(data)
    ok = kernel32.WriteProcessMemory(
        process_handle,
        ctypes.c_void_p(address),
        buffer,
        len(data),
        ctypes.byref(written),
    )
    if not ok or written.value != len(data):
        fail_last_error(f"WriteProcessMemory failed at 0x{address:08X}")


def patch_antidebug(process_handle: wintypes.HANDLE) -> None:
    # xor eax,eax; ret; nop; nop; nop
    write_memory(process_handle, CHECK_DEBUGGER, b"\x31\xC0\xC3\x90\x90\x90")
    # mov eax,0; any inline anti-debug branch landing here becomes "not detected".
    write_memory(process_handle, INLINE_DETECTED_RESULT, b"\xB8\x00\x00\x00\x00")


def patch_entry_break(process_handle: wintypes.HANDLE) -> None:
    write_memory(process_handle, ENTRY_POINT, b"\xCC")


def create_suspended(target: Path) -> PROCESS_INFORMATION:
    startup = STARTUPINFO()
    startup.cb = ctypes.sizeof(startup)
    info = PROCESS_INFORMATION()
    command_line = subprocess.list2cmdline([str(target)])

    ok = kernel32.CreateProcessW(
        str(target),
        command_line,
        None,
        None,
        False,
        CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
        None,
        str(target.parent),
        ctypes.byref(startup),
        ctypes.byref(info),
    )
    if not ok:
        fail_last_error("CreateProcessW failed")
    return info


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Create crackme.exe as a suspended process for x32dbg attach."
    )
    parser.add_argument(
        "target",
        nargs="?",
        default="crackme.exe",
        help="target exe path; default: .\\crackme.exe",
    )
    parser.add_argument(
        "--no-patch",
        action="store_true",
        help="do not patch checkDebugger before attach",
    )
    parser.add_argument(
        "--break-entry",
        action="store_true",
        help="write int3 at 0x4012E0 before attach so x32dbg breaks at entry",
    )
    args = parser.parse_args()

    if os.name != "nt":
        print("Run this script with Windows Python, not WSL/Linux Python.")
        return 2

    target = Path(args.target).resolve()
    if not target.is_file():
        print(f"Target not found: {target}")
        return 1

    info = create_suspended(target)
    try:
        if not args.no_patch:
            patch_antidebug(info.hProcess)
            print(f"Patched checkDebugger at 0x{CHECK_DEBUGGER:08X}: xor eax,eax; ret")
            print(f"Patched inline anti-debug result at 0x{INLINE_DETECTED_RESULT:08X}: mov eax,0")
        if args.break_entry:
            patch_entry_break(info.hProcess)
            print(f"Patched entry point at 0x{ENTRY_POINT:08X}: int3")

        print(f"Created suspended process: PID={info.dwProcessId}, TID={info.dwThreadId}")
        print("x32dbg: File -> Attach -> select this PID.")
        print("After x32dbg attaches, return here and press Enter to resume.")
        print("If --break-entry was used, x32dbg should break at 0x4012E0 after resume.")
        print("If you do not want to resume, type /kill and press Enter.")

        answer = input("> ").strip().lower()
        if answer == "/kill":
            kernel32.TerminateProcess(info.hProcess, 1)
            print("Terminated.")
            return 1

        previous_count = kernel32.ResumeThread(info.hThread)
        if previous_count == 0xFFFFFFFF:
            fail_last_error("ResumeThread failed")
        print("Primary thread resumed. Continue in x32dbg.")

        kernel32.WaitForSingleObject(info.hProcess, INFINITE)
        code = process_exit_code(info.hProcess)
        print_exit_status(code)
        return code
    finally:
        kernel32.CloseHandle(info.hThread)
        kernel32.CloseHandle(info.hProcess)


if __name__ == "__main__":
    raise SystemExit(main())
