#undef SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC "\
%{n64-wiivc:-DWIIVC}"

#undef SUBTARGET_CC1_SPEC
#define SUBTARGET_CC1_SPEC "\
%{n64-wiivc:%{O0:-O0; -Og:-Og; O*:-O1} %<O* %{!freorder-blocks:-fno-reorder-blocks}} \
%{n64-dynamic:%{!G*:-G0} %{!mgpopt:-mno-gpopt}}"
