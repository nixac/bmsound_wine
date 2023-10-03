##    test-client.bin    ##
$(call generic_target,test-client,c)

# Local #define
$(call incl_define,$(target),)

# Local includes (#include "")
$(call incl_target_dirs,$(target))
$(call incl_quoted,$(target),)

# External includes (#include <>)
$(call incl_angled,$(target),)

# Local dynamic linker (compile-able libraries)
$(call link_reference,$(target),bmsound-pw)

# External dynamic linker (binary libraries) - external/system libraries
$(call link_package,$(target),)
$(call link_external,$(target),)

$($(target)_build)/%.o:		$(target)@pre $($(target)_src)
	$(call gcc_object,test-client,$*)

$(target):					$($(target)_obj)
	$(call gcc_executable,test-client)
