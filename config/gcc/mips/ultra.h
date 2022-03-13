#undef SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC "\
%{n64-wiivc:-DWIIVC}"

#undef SUBTARGET_CC1_SPEC
#define SUBTARGET_CC1_SPEC "\
%{n64-wiivc:%{!freorder-blocks:-fno-reorder-blocks} %{!foptimize-sibling-calls:-fno-optimize-sibling-calls}} \
%{n64-dynamic:%{!G*:-G0} %{!mgpopt:-mno-gpopt}} \
%{!mcheck-zero-division:%{!mno-check-zero-division:%{!mdivide-traps:%{!mdivide-breaks:-mno-check-zero-division}}}}"
