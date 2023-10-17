##    bmsound-wine.{dll:so}    ##
$(call generic_target,bmsound-wine,c)

# Local #define
$(call incl_define,$(target),)

# Local includes (#include "")
$(call incl_target_dirs,$(target))
$(call incl_quoted,$(target),)

# External includes (#include <>)
$(call incl_angled,$(target),$(OUTPUT_DIR)/include)

# Local dynamic linker (compile-able libraries)
$(call link_reference,$(target),bmsound-pw)

# External dynamic linker (binary libraries) - external/system libraries
$(call link_package,$(target),)
$(call link_external,$(target),)

$($(target)_build)/%.o:		$(target)@pre $($(target)_src)
	$(call wine_object,bmsound-wine,$*)

$(target):					$($(target)_obj)
	$(call wine_library,bmsound-wine)
