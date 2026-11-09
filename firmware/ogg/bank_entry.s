// SPDX-License-Identifier: BSD-2-Clause
.text
.global codec_bank_command
codec_bank_command:
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    call codec_bank_command_impl, always
    pop mod0
    ret always

.global codec_bank_poll
codec_bank_poll:
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    call codec_bank_poll_impl, always
    pop mod0
    ret always
