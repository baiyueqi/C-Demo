
; =============== S U B R O U T I N E =======================================

; Attributes: bp-based frame

; int __cdecl process_user_data(const char *username, const char *data)
?process_user_data@@YAHPBD0@Z proc near ; CODE XREF: business_logic_handler(char const *)+A6↓p

user_buffer     = byte ptr -48h
is_admin        = dword ptr -8
var_4           = dword ptr -4
username        = dword ptr  8
data            = dword ptr  0Ch

                push    ebp
                mov     ebp, esp
                sub     esp, 48h
                mov     [ebp+is_admin], 0
                cmp     [ebp+username], 0
                jz      short loc_4011AF
                cmp     [ebp+data], 0
                jz      short loc_4011AF
                push    offset String2  ; "Admin"
                mov     eax, [ebp+username]
                push    eax             ; String1
                call    ds:__imp___stricmp
                add     esp, 8
                test    eax, eax
                jnz     short loc_401178
                mov     [ebp+var_4], 1
                jmp     short loc_40117F
; ---------------------------------------------------------------------------

loc_401178:                             ; CODE XREF: process_user_data(char const *,char const *)+2D↑j
                mov     [ebp+var_4], 0

loc_40117F:                             ; CODE XREF: process_user_data(char const *,char const *)+36↑j
                mov     ecx, [ebp+var_4]
                mov     [ebp+is_admin], ecx
                mov     edx, [ebp+data]
                push    edx
                mov     eax, [ebp+username]
                push    eax
                push    offset _Format  ; "User=%s, Data=%s"
                lea     ecx, [ebp+user_buffer]
                push    ecx             ; _Buffer
                call    _sprintf
                add     esp, 10h
                lea     edx, [ebp+user_buffer]
                push    edx
                push    offset aProcessingS ; "Processing: %s\n"
                call    _printf
                add     esp, 8

loc_4011AF:                             ; CODE XREF: process_user_data(char const *,char const *)+11↑j
                                        ; process_user_data(char const *,char const *)+17↑j
                mov     eax, [ebp+is_admin]
                mov     esp, ebp
                pop     ebp
                retn
?process_user_data@@YAHPBD0@Z endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: bp-based frame

; int __cdecl business_logic_handler(const char *user)
?business_logic_handler@@YAHPBD@Z proc near ; CODE XREF: _main+5E↓p

file            = byte ptr -208h
buffer          = byte ptr -108h
result          = dword ptr -8
fp              = dword ptr -4
user            = dword ptr  8

                push    ebp
                mov     ebp, esp
                sub     esp, 208h
                push    100h            ; Size
                push    0               ; Val
                lea     eax, [ebp+buffer]
                push    eax             ; void *
                call    _memset
                add     esp, 0Ch
                mov     [ebp+result], 0
                mov     ecx, [ebp+user]
                push    ecx
                push    offset aSDataTxt ; "%s_data.txt"
                lea     edx, [ebp+file]
                push    edx             ; _Buffer
                call    _sprintf
                add     esp, 0Ch
                push    offset Mode     ; "r"
                lea     eax, [ebp+file]
                push    eax             ; FileName
                call    ds:__imp__fopen
                add     esp, 8
                mov     [ebp+fp], eax
                cmp     [ebp+fp], 0
                jnz     short loc_401235
                lea     ecx, [ebp+buffer]
                push    ecx
                push    offset aOpenSError ; "open %s error\n"
                call    _printf
                add     esp, 8
                or      eax, 0FFFFFFFFh
                jmp     short loc_401298
; ---------------------------------------------------------------------------

loc_401235:                             ; CODE XREF: business_logic_handler(char const *)+5A↑j
                mov     edx, [ebp+fp]
                push    edx             ; Stream
                push    0FFh            ; MaxCount
                lea     eax, [ebp+buffer]
                push    eax             ; Buffer
                call    ds:__imp__fgets
                add     esp, 0Ch
                mov     ecx, [ebp+fp]
                push    ecx             ; Stream
                call    ds:__imp__fclose
                add     esp, 4
                lea     edx, [ebp+buffer]
                push    edx             ; data
                mov     eax, [ebp+user]
                push    eax             ; username
                call    ?process_user_data@@YAHPBD0@Z ; process_user_data(char const *,char const *)
                add     esp, 8
                mov     [ebp+result], eax
                cmp     [ebp+result], 19224h
                jnz     short loc_401295
                push    offset aAdminPrivilege ; "Admin privileges activated!\n"
                call    _printf
                add     esp, 4
                push    offset Command  ; "cmd.exe"
                call    ds:__imp__system
                add     esp, 4

loc_401295:                             ; CODE XREF: business_logic_handler(char const *)+B8↑j
                mov     eax, [ebp+result]

loc_401298:                             ; CODE XREF: business_logic_handler(char const *)+73↑j
                mov     esp, ebp
                pop     ebp
                retn
?business_logic_handler@@YAHPBD@Z endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: bp-based frame

; int __cdecl main(int argc, const char **argv, const char **envp)
_main           proc near               ; CODE XREF: __scrt_common_main_seh+F5↓p

user            = dword ptr -4
argc            = dword ptr  8
argv            = dword ptr  0Ch
envp            = dword ptr  10h

                push    ebp
                mov     ebp, esp
                push    ecx
                mov     [ebp+user], offset aTest ; "test"
                push    offset aWindowsBufferO ; "Windows Buffer Overflow Demonstration\n"
                call    _printf
                add     esp, 4
                push    offset asc_4021C4 ; "=====================================\n"
                call    _printf
                add     esp, 4
                push    offset aUsageSsexVulId ; "Usage: SSEx_vul ID\n\n"
                call    _printf
                add     esp, 4
                cmp     [ebp+argc], 2
                jnz     short loc_4012E9
                mov     eax, 4
                shl     eax, 0
                mov     ecx, [ebp+argv]
                mov     edx, [ecx+eax]
                mov     [ebp+user], edx

loc_4012E9:                             ; CODE XREF: _main+36↑j
                mov     eax, [ebp+user]
                push    eax
                push    offset aUserS   ; "user: %s\n"
                call    _printf
                add     esp, 8
                mov     ecx, [ebp+user]
                push    ecx             ; user
                call    ?business_logic_handler@@YAHPBD@Z ; business_logic_handler(char const *)
                add     esp, 4
                xor     eax, eax
                mov     esp, ebp
                pop     ebp
                retn
_main           endp

