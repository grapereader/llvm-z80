;RUN: rm -f %T/test.a
;RUN: not llvm-ar r %T/test.a . 2>&1 | FileCheck %s
;CHECK: .: Is a directory

; Opening a directory works on cygwin and freebsd.
;XFAIL: freebsd, cygwin

;RUN: rm -f %T/test.a
;RUN: touch %T/a-very-long-file-name
;RUN: llvm-ar r %T/test.a %s %T/a-very-long-file-name
;RUN: llvm-ar r %T/test.a %T/a-very-long-file-name
;RUN: llvm-ar t %T/test.a | FileCheck -check-prefix=MEMBERS %s
;MEMBERS-NOT: /
;MEMBERS: directory.ll
;MEMBERS: a-very-long-file-name
;MEMBERS-NOT: a-very-long-file-name
