##    bmsound-pw.so    ##
$(call generic_target,bmsound-pw,c)

# Local #define
$(call incl_define,$(target),)

# Local includes (#include "")
$(call incl_target_dirs,$(target))
$(call incl_quoted,$(target),)

# External includes (#include <>)
$(call incl_angled,$(target),)

# Local dynamic linker (compile-able libraries)
$(call link_reference,$(target),)

# External dynamic linker (binary libraries) - external/system libraries
$(call link_package,$(target),libspa-0.2 libpipewire-0.3)
$(call link_external,$(target),)

$($(target)_build)/%.o:		$(target)@pre $($(target)_src)
	$(call gcc_object,bmsound-pw,$*)

$(target):					$($(target)_obj)
	$(call gcc_library,bmsound-pw)
